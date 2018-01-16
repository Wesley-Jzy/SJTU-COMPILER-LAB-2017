#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "liveness.h"
#include "color.h"
#include "regalloc.h"
#include "table.h"
#include "flowgraph.h"

#define K 6
#define MAXLINE 40

static void Build();
static void AddEdge(G_node u, G_node v);
static void MakeWorklist();
static G_nodeList Adjacent(G_node n);
static Live_moveList NodeMoves(G_node n);
static bool MoveRelated(G_node n);
static void Simplify();
static void DecrementDegree(G_node m);
static void EnableMoves(G_nodeList nodes);
static void Coalesce();
static void AddWorkList(G_node u);
static bool OK(G_node t, G_node r);
static bool Conservative(G_nodeList nodes);
static G_node GetAlias(G_node n);
static void Combine(G_node u, G_node v);
static void Freeze();
static void FreezeMoves(G_node u);
static void SelectSpill();
static void AssignColors();
static void RewriteProgram(F_frame f, AS_instrList *pil);
static Temp_map AssignRegisters(struct Live_graph g);

static bool inTempList(Temp_tempList a, Temp_temp t);
static Temp_tempList replaceTempList(Temp_tempList l, Temp_temp old, Temp_temp new);
static bool inMoveList(Live_moveList a, G_node src, G_node dst);
static Live_moveList unionMoveList(Live_moveList a, Live_moveList b);
static Live_moveList subMoveList(Live_moveList a, Live_moveList b);
static bool precolored(G_node n);
static Temp_tempList* Inst_def(AS_instr inst);
static Temp_tempList* Inst_use(AS_instr inst);

static char *hard_regs[7] = {"none", "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi"};
static G_nodeList simplifyWorklist;
static G_nodeList freezeWorklist;
static G_nodeList spillWorklist;

static G_nodeList spilledNodes;
static G_nodeList coalescedNodes;
static G_nodeList coloredNodes;
static G_nodeList selectStack;

static Live_moveList coalescedMoves;
static Live_moveList constrainedMoves;
static Live_moveList frozenMoves;
static Live_moveList worklistMoves;
static Live_moveList activeMoves;

static Temp_tempList notSpillTemps;

static G_table degreeTab;
static G_table colorTab;
static G_table aliasTab;

static struct Live_graph liveGraph;

struct RA_result RA_regAlloc(F_frame f, AS_instrList il) {
	//printf("RA_regAlloc\n");
	struct RA_result res;

	bool finish = FALSE;
	int count = 0;
	while (!finish) {
		printf("--------------Ra:%d\n", count++);
		//printf("RA_loop\n");
		finish =TRUE;

		/* LivenessAnalysis() */
		G_graph flowGraph = FG_AssemFlowGraph(il, f);
		liveGraph = Live_liveness(flowGraph);

		//printf("after liveness\n");

		/* Build() */
		Build();

		//printf("after build\n");

		/* MakeWorklist() */
		MakeWorklist();

		//printf("after makeWorklist\n");

		/* repeat until all workLists = {} */
		while (simplifyWorklist || worklistMoves || freezeWorklist || spillWorklist) {
			/* if simlifyWorklisk != {} then Simlify() */
			/* if worklistMoves != {} then Coalesce() */
			/* if freezeWorklist != {} then Freeze() */
			/* else if spillWorklist != {} then SelectSpill() */
			if (simplifyWorklist) {
			    Simplify();
			} else if (worklistMoves) {
			    Coalesce();
			} else if (freezeWorklist) {
			    Freeze();
			} else if (spillWorklist) {
			    SelectSpill();
			}
		}

		//printf("after loop\n");

		/* AssignColor() */
		AssignColors();

		//printf("after assignColors\n");

		/* if spilledNodes != {} then */
		if (spilledNodes) {
			printf("spilledNodes:\n");
			int count2 = 0;
			for (G_nodeList p = spilledNodes;p; p = p->tail) {
				Temp_temp t = G_nodeInfo(p->head);
				printf("node%d: t%d\n", count2++, Temp_int(t));
			}
			/* RewriteProgram(spilledNodes) */
			RewriteProgram(f, &il);
			/* Main */
			finish = FALSE;
			//printf("after rewriteProgram\n");
		}
	}

	
	res.coloring = AssignRegisters(liveGraph);
	//printf("after assignRegisters\n");
	res.il = il;

	return res;
}

static void Build() {
	/* initial */
	simplifyWorklist = NULL;
	freezeWorklist = NULL;
	spillWorklist = NULL;

	spilledNodes = NULL;
	coalescedNodes = NULL;
	coloredNodes = NULL;
	selectStack = NULL;

	coalescedMoves = NULL;
	constrainedMoves = NULL;
	frozenMoves = NULL;
	worklistMoves = liveGraph.moves;
	activeMoves = NULL;

	degreeTab = G_empty();
	colorTab = G_empty();
	aliasTab = G_empty();

	G_nodeList nodes = G_nodes(liveGraph.graph);
	for (; nodes; nodes = nodes->tail) {
		/* initial degree */
		int *degree = checked_malloc(sizeof(int));
		*degree = 0;
		for (G_nodeList cur = G_succ(nodes->head); cur; cur = cur->tail) {
			(*degree)++;
		}
		G_enter(degreeTab, nodes->head, degree);

		/* intial color */
		int *color = checked_malloc(sizeof(int));
		Temp_temp temp = Live_gtemp(nodes->head);
		if (temp == F_EAX()) {
			*color = 1;
		} else if (temp == F_EBX()) {
			*color = 2;
		} else if (temp == F_ECX()) {
			*color = 3;
		} else if (temp == F_EDX()) {
			*color = 4;
		} else if (temp == F_ESI()) {
			*color = 5;
		} else if (temp == F_EDI()) {
			*color = 6;
		} else {
			*color = 0;
		}

		G_enter(colorTab, nodes->head, color);

		/* initial alias */
		G_node *alias = checked_malloc(sizeof(G_node));
		*alias = nodes->head;
		G_enter(aliasTab, nodes->head, alias);
	}
}

static void AddEdge(G_node u, G_node v) {
	if (!G_inNodeList(u, G_adj(v)) && u != v) {
		if (!precolored(u)) {
			(*(int *)G_look(degreeTab, u))++;
			G_addEdge(u, v);
		}
		if (!precolored(v)) {
			(*(int *)G_look(degreeTab, v))++;
			G_addEdge(v, u);
		}
	}
}

static void MakeWorklist() {
	G_nodeList nodes = G_nodes(liveGraph.graph);
	for (; nodes; nodes = nodes->tail) {
		int *degree = G_look(degreeTab, nodes->head);
		int *color = G_look(colorTab, nodes->head);

		if (*color != 0) {
			continue;
		}

		if (*degree >= K) { /* spill */
			spillWorklist = G_NodeList(nodes->head, spillWorklist);
		} else if (MoveRelated(nodes->head)) { /* moveRelated */
			freezeWorklist = G_NodeList(nodes->head, freezeWorklist);
		} else { /* simplify */
			simplifyWorklist = G_NodeList(nodes->head, simplifyWorklist);
		}
	}
}

static G_nodeList Adjacent(G_node n) {
	return G_subNodeList(G_subNodeList(G_succ(n), selectStack), coalescedNodes);
}

static Live_moveList NodeMoves(G_node n) {
	Live_moveList moves = NULL;
	G_node m = GetAlias(n);
	for (Live_moveList p = unionMoveList(activeMoves, worklistMoves); p; p = p->tail) {
		if (GetAlias(p->src) == m || GetAlias(p->dst) == m) {
			moves = Live_MoveList(p->src, p->dst, moves);
		}
	}
	return moves;
}

static bool MoveRelated(G_node n) {
	return (NodeMoves(n) != NULL);
}

static void Simplify() {
	G_node n = simplifyWorklist->head;
	simplifyWorklist = simplifyWorklist->tail;
	selectStack = G_NodeList(n, selectStack);
	for (G_nodeList nodes = Adjacent(n); nodes; nodes = nodes->tail) {
		DecrementDegree(nodes->head);
	}
}

static void DecrementDegree(G_node m) {
	int *degree = G_look(degreeTab, m);
	int d = *degree;
	(*degree)--;

	int *color = G_look(colorTab, m);
	if (d == K && *color == 0) {
		EnableMoves(G_NodeList(m, Adjacent(m)));
		spillWorklist = G_subNodeList(spillWorklist, G_NodeList(m, NULL));
		if (MoveRelated(m)) {
			freezeWorklist = G_NodeList(m, freezeWorklist);
		} else {
			simplifyWorklist = G_NodeList(m, simplifyWorklist);
		}
	}
}

static void EnableMoves(G_nodeList nodes) {
	for (; nodes; nodes = nodes->tail) {
		G_node n = nodes->head;
		for (Live_moveList m = NodeMoves(n); m; m = m->tail) {
			if (inMoveList(activeMoves, m->src, m->dst)) {
				activeMoves = subMoveList(activeMoves, Live_MoveList(m->src, m->dst, NULL));
				worklistMoves = Live_MoveList(m->src, m->dst, worklistMoves);
			}
		}
	}
}

static void Coalesce() {
	G_node u, v;
	G_node x = worklistMoves->src;
	G_node y = worklistMoves->dst;
	if (precolored(GetAlias(y))) {
		u = GetAlias(y);
		v = GetAlias(x);
	} else {
		u = GetAlias(x);
		v = GetAlias(y);
	}
	worklistMoves = worklistMoves->tail;
	if (u == v) {
		coalescedMoves = Live_MoveList(x, y, coalescedMoves);
		AddWorkList(u);
	} else if (precolored(v) || G_inNodeList(u, G_adj(v))) {
		constrainedMoves = Live_MoveList(x, y, constrainedMoves);
		AddWorkList(u);
		AddWorkList(v);
	} else if ((precolored(u) && (OK(v, u))) 
		      || (!precolored(u) && Conservative(G_unionNodeList(Adjacent(u), Adjacent(v))))) {
		coalescedMoves = Live_MoveList(x, y, coalescedMoves);
		Combine(u, v);
		AddWorkList(u);
	} else {
		activeMoves = Live_MoveList(x, y, activeMoves);
	}
}

static void AddWorkList(G_node u) {
	if (!precolored(u) && !MoveRelated(u) && *(int *)G_look(degreeTab, u) < K) {
		freezeWorklist = G_subNodeList(freezeWorklist, G_NodeList(u, NULL));
		simplifyWorklist = G_NodeList(u, simplifyWorklist);
	}
}

static bool OK(G_node t, G_node r) {
	bool ok = TRUE;
	for (G_nodeList p = Adjacent(t); p; p = p->tail) {
		if(*(int *)G_look(degreeTab, p->head) < K || precolored(p->head) || G_inNodeList(p->head, G_adj(r))) {
			ok = TRUE;
		} else {
			return FALSE;
		}
	}

	return ok;
}

static bool Conservative(G_nodeList nodes) {
	int k = 0;
	for (; nodes; nodes = nodes->tail) {
		G_node n = nodes->head;
		if (*(int *)G_look(degreeTab, n) >= K) {
			k++;
		}
	}

	return (k < K);
}

static G_node GetAlias(G_node n) {
	G_node *res = G_look(aliasTab, n);
    if (*res != n) {
    	*res = GetAlias(*res);
    }
    return *res;
}

static void Combine(G_node u, G_node v) {
	if (G_inNodeList(v, freezeWorklist)) {
		freezeWorklist = G_subNodeList(freezeWorklist, G_NodeList(v, NULL));
	} else {
		spillWorklist = G_subNodeList(spillWorklist, G_NodeList(v, NULL));
	}
	coalescedNodes = G_NodeList(v, coalescedNodes);
	*(G_node *)G_look(aliasTab, v) = u;
	for (G_nodeList t = Adjacent(v); t; t = t->tail) {
		AddEdge(t->head, u);
		DecrementDegree(t->head);
	}
	if (*(int *)G_look(degreeTab, u) >= K && G_inNodeList(u, freezeWorklist)) {
		freezeWorklist = G_subNodeList(freezeWorklist, G_NodeList(u, NULL));
		spillWorklist = G_NodeList(u, spillWorklist);
	}
}

static void Freeze() {
	G_node u = freezeWorklist->head;
	freezeWorklist = freezeWorklist->tail;
	simplifyWorklist = G_NodeList(u, simplifyWorklist);
	FreezeMoves(u);
}

static void FreezeMoves(G_node u) {
	for (Live_moveList m = NodeMoves(u); m; m = m->tail) {
		G_node x = m->src;
		G_node y = m->dst;
		G_node v;
		if (GetAlias(y) == GetAlias(u)) {
			v = GetAlias(x);
		} else {
			v = GetAlias(y);
		}
		activeMoves = subMoveList(activeMoves, Live_MoveList(x, y, NULL));
		frozenMoves = Live_MoveList(x, y, frozenMoves);
		if (NodeMoves(v) == NULL && *(int *)G_look(degreeTab, v) < K) {
			freezeWorklist = G_subNodeList(freezeWorklist, G_NodeList(v, NULL));
			simplifyWorklist = G_NodeList(v, simplifyWorklist);
		}
	}
}

static void SelectSpill() {
	G_node m = NULL;
	int max = 0;
	for (G_nodeList p = spillWorklist; p; p = p->tail) {
		Temp_temp t = G_nodeInfo(p->head);
		if (inTempList(notSpillTemps, t)) {
			continue;
		}
		int degree = *(int *)G_look(degreeTab, p->head);
		if (degree > max) {
			max = degree;
			m = p->head;
		}
	}

	if (m) {
		spillWorklist = G_subNodeList(spillWorklist, G_NodeList(m, NULL));
		simplifyWorklist = G_NodeList(m, simplifyWorklist);
		FreezeMoves(m);
	} else {
		m = spillWorklist->head;
		spillWorklist = spillWorklist->tail;
		simplifyWorklist = G_NodeList(m, simplifyWorklist);
		FreezeMoves(m);
	}
}

static void AssignColors() {
	bool okColors[K + 1];

	spilledNodes = NULL;
	while (selectStack) {
		okColors[0] = FALSE;
		for (int i = 1; i < K + 1; i++) {
			okColors[i] = TRUE;
		}

		G_node n = selectStack->head;
		selectStack = selectStack->tail;
		for (G_nodeList succs = G_succ(n); succs; succs = succs->tail) {
			int *color = G_look(colorTab, GetAlias(succs->head));
			okColors[*color] = FALSE;
		}

		int i;
		bool realSpill = TRUE;
		for (i = 1; i < K + 1; i++) {
			if (okColors[i]) {
				realSpill = FALSE;
				break;
			}
		}

		if (realSpill) {
			spilledNodes = G_NodeList(n, spilledNodes);
		} else {
			int *color = G_look(colorTab, n);
			*color = i;
		}
	}

	for (G_nodeList p = G_nodes(liveGraph.graph); p; p = p->tail) {
		*(int *)G_look(colorTab, p->head) = *(int *)G_look(colorTab, GetAlias(p->head));
	}
}

static void RewriteProgram(F_frame f, AS_instrList *pil) {
	notSpillTemps = NULL;
    AS_instrList il = *pil, l, last, next, new_instr;
    int off;

    while(spilledNodes) {
        G_node cur = spilledNodes->head;
        spilledNodes = spilledNodes->tail;
        Temp_temp c = Live_gtemp(cur);

        off = F_spill(f);

        l = il;
        last = NULL;
        while(l) {
            Temp_temp t = NULL;
            next = l->tail;
            Temp_tempList *def = Inst_def(l->head);
            Temp_tempList *use = Inst_use(l->head);
            if (use && inTempList(*use, c)) {
                if (t == NULL) {
                    t = Temp_newtemp();
                    notSpillTemps = Temp_TempList(t, notSpillTemps);
                    printf("load-rewrite:t%d\n", Temp_int(t));
                }
                *use = replaceTempList(*use, c, t);
                char *a = checked_malloc(MAXLINE * sizeof(char));
                sprintf(a, "movl %d(%%ebp), `d0\n", off);
                new_instr = AS_InstrList(AS_Oper(a, Temp_TempList(t, NULL), NULL, AS_Targets(NULL), FALSE), l);
                if (last) {
                    last->tail = new_instr;
                } else {
                    il = new_instr;
                }
            }
            last = l;
            if (def && inTempList(*def, c)) {
                if (t == NULL) {
                    t = Temp_newtemp();
                    notSpillTemps = Temp_TempList(t, notSpillTemps);
                    printf("store-rewrite:t%d\n", Temp_int(t));
                }
                *def = replaceTempList(*def, c, t);
                char *a = checked_malloc(MAXLINE * sizeof(char));
                sprintf(a, "movl `s0, %d(%%ebp)\n", off);
                l->tail = AS_InstrList(AS_Oper(a, NULL, Temp_TempList(t, NULL), AS_Targets(NULL), FALSE), next);
                last = l->tail;
            }
            l = next;
        }
    }
    *pil = il;
}

static Temp_map AssignRegisters(struct Live_graph g) {
	Temp_map res = Temp_empty();
	G_nodeList nodes = G_nodes(g.graph);

	Temp_enter(res, F_FP(), "%ebp");
	
	for (; nodes; nodes = nodes->tail) {
		int *color = G_look(colorTab, nodes->head);
		Temp_enter(res, Live_gtemp(nodes->head), hard_regs[*color]);
	}

	return res;
}

static Temp_tempList replaceTempList(Temp_tempList l, Temp_temp old, Temp_temp new) {
  if (l) {
    if (l->head == old) {
      return Temp_TempList(new, replaceTempList(l->tail, old, new));
    } else {
      return Temp_TempList(l->head, replaceTempList(l->tail, old, new));
    }
  } else {
    return NULL;
  }
}

static bool inTempList(Temp_tempList a, Temp_temp t) {
	bool in = FALSE;

	for (; a; a = a->tail) {
		if (a->head == t) {
			in = TRUE;
			return in;
		}
	}

	return in;
}

static bool inMoveList(Live_moveList a, G_node src, G_node dst) {
	bool in = FALSE;

	for (; a; a = a->tail) {
		if (a->src == src && a->dst == dst) {
			in = TRUE;
			return in;
		}
	}

	return in;
}

static Live_moveList unionMoveList(Live_moveList a, Live_moveList b) {
    Live_moveList res = a;
    for (Live_moveList p = b; p; p = p->tail) {
        if (!inMoveList(a, p->src, p->dst)) {
            res = Live_MoveList(p->src, p->dst, res);
        }
    }
    return res;
}

static Live_moveList subMoveList(Live_moveList a, Live_moveList b) {
    Live_moveList res = NULL;
    for (Live_moveList p = a; p; p = p->tail) {
        if (!inMoveList(b, p->src, p->dst)) {
            res = Live_MoveList(p->src, p->dst, res);
        }
    }
    return res;
}

static bool precolored(G_node n) {
	return *(int *)G_look(colorTab, n);
}

static Temp_tempList* Inst_def(AS_instr inst) {
	switch (inst->kind) {
		case I_OPER:
			return &inst->u.OPER.dst;
		case I_LABEL:
			return NULL;
		case I_MOVE:
			return &inst->u.MOVE.dst;
    }
	return NULL;
}

static Temp_tempList* Inst_use(AS_instr inst) {
	switch (inst->kind) {
		case I_OPER:
			return &inst->u.OPER.src;
		case I_LABEL:
			return NULL;
		case I_MOVE:
			return &inst->u.MOVE.src;

	}
	return NULL;
}