#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"
#include "table.h"

static Temp_tempList unionTempList(Temp_tempList a, Temp_tempList b);
static Temp_tempList subTempList(Temp_tempList a, Temp_tempList b);
static bool isEqual(Temp_tempList a, Temp_tempList b);
static bool inTempList(Temp_tempList a, Temp_temp t);
static bool inMoveList(Live_moveList a, G_node src, G_node dst);


Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail) {
	Live_moveList lm = (Live_moveList) checked_malloc(sizeof(*lm));
	lm->src = src;
	lm->dst = dst;
	lm->tail = tail;
	return lm;
}


Temp_temp Live_gtemp(G_node n) {
	Temp_temp t = G_nodeInfo(n);
	return t;
}

void printTempList(Temp_tempList p, char *name) {
	if (!p) {
		printf("%s = NULL\n", name);
	} else {
		printf("%s = ", name);
		for (; p; p = p->tail) {
			printf("%d->", Temp_int(p->head));
		}
		printf("\n");
	}
}

struct Live_graph Live_liveness(G_graph flow) {
	struct Live_graph lg;

	/* Get all nodes in flow */
	G_nodeList nodes = G_nodes(flow);

	G_table inTab = G_empty();
	G_table outTab = G_empty();
	/* for each n */
	for (; nodes; nodes = nodes->tail) {
		/* init in, out */
		G_enter(inTab, nodes->head, checked_malloc(sizeof(Temp_tempList)));
		G_enter(outTab, nodes->head, checked_malloc(sizeof(Temp_tempList)));
	}

	bool finish = FALSE;
	/* until in'[n] = in[n] and out'[n] = out[n] for each n */
	// int count0 = 0;
	while(!finish) {
		// count0++;
		// printf("loop:%d\n", count0);
		finish = TRUE;
		nodes = G_nodes(flow);
		/* for each n */
		G_node n = NULL;
		//int count1 = 0;
		for (; nodes; nodes = nodes->tail) {
			// count1++;
			// printf("node%d\n", count1);
			n = nodes->head;

			/* in'[n] <- in[n] */
			Temp_tempList in_old = *(Temp_tempList *)G_look(inTab, n);			

			/* out'[n] <- out[n] */
			Temp_tempList out_old = *(Temp_tempList *)G_look(outTab, n);

			Temp_tempList in = NULL, out = NULL;

			/* out[n] <- all in[succ(n)]'s Union */
			G_nodeList succs = G_succ(n);
			for (; succs; succs = succs->tail) {
				Temp_tempList in_succ = *(Temp_tempList *)G_look(inTab, succs->head);
				out = unionTempList(out, in_succ);
			}

			/* in[n] <- use[n] U (out[n] - def[n]) */
			in = unionTempList(FG_use(n), subTempList(out, FG_def(n)));

			/* in[n] & out[n] not change? */
			if (!isEqual(in_old, in) || !isEqual(out_old, out)) {
				finish = FALSE;
			}

			// printTempList(in, "in");
			// printTempList(in_old, "in_old");
			// printTempList(out, "out");
			// printTempList(out_old, "out_old");

			/* save in and out */
			*(Temp_tempList*)G_look(inTab, n) = in;
			*(Temp_tempList*)G_look(outTab, n) = out;
		}
	}

	/* Construct conflict graph */
	lg.moves = NULL;
	lg.graph = G_Graph();

	TAB_table tempTab= TAB_empty();
	/* enter hard registers and addEdge */
	Temp_tempList hardRegs = 
			Temp_TempList(F_EAX(),
			Temp_TempList(F_EBX(),
			Temp_TempList(F_ECX(),
			Temp_TempList(F_EDX(),
			Temp_TempList(F_ESI(),
			Temp_TempList(F_EDI(), NULL))))));

	for (Temp_tempList temps = hardRegs; temps; temps = temps->tail) {
		G_node tempNode = G_Node(lg.graph, temps->head);
		TAB_enter(tempTab, temps->head, tempNode);
	}

	for (Temp_tempList temps = hardRegs; temps; temps = temps->tail) {
		for (Temp_tempList next = temps->tail; next; next = next->tail) {
			G_node a = TAB_look(tempTab, temps->head);
			G_node b = TAB_look(tempTab, next->head);
			G_addEdge(a, b);
			G_addEdge(b, a);
		}
	}

	/* temp registers */
	/* enter temp registers */
	nodes = G_nodes(flow);
	for (; nodes; nodes = nodes->tail) {
		G_node n = nodes->head;
		Temp_tempList def = FG_def(n);
		Temp_tempList out = *(Temp_tempList *)G_look(outTab, n);

		if (!(def && def->head)) {
			continue;
		}
		for (; def; def = def->tail) {
			if (def->head == F_FP()) {
				continue;
			}
			if (!TAB_look(tempTab, def->head)) {
				G_node tempNode = G_Node(lg.graph, def->head);
				TAB_enter(tempTab, def->head, tempNode);
			}
		}

		for (; out; out = out->tail) {
			if (out->head == F_FP()) {
				continue;
			}
			if (!TAB_look(tempTab, out->head)) {
				G_node tempNode = G_Node(lg.graph, out->head);
				TAB_enter(tempTab, out->head, tempNode);
			}
		}
	}

	/* addEdge */
	nodes = G_nodes(flow);
	for (; nodes; nodes = nodes->tail) {
		G_node n = nodes->head;

		if (!FG_isMove(n)) {
			Temp_tempList def = FG_def(n);
			Temp_tempList outhead = *(Temp_tempList *)G_look(outTab, n);

			for (; def; def = def->tail) {
				if (def->head == F_FP()) {
					continue;
				}

				G_node a = TAB_look(tempTab, def->head);

				for (Temp_tempList out = outhead; out; out = out->tail) {
					if (out->head == F_FP() || out->head == def->head) {
						continue;
					}

					G_node b = TAB_look(tempTab, out->head);
					if (!G_inNodeList(a, G_adj(b))) {
						//printf("t%d---", Temp_int(def->head));
						//printf("t%d\n", Temp_int(out->head));
						G_addEdge(a, b);
						G_addEdge(b, a);
					}
				}
			}
		} else {
			Temp_tempList def = FG_def(n);
			Temp_tempList use = FG_use(n);
			Temp_tempList outhead = *(Temp_tempList *)G_look(outTab, n);

			for (; def; def = def->tail) {
				if (def->head == F_FP()) {
					continue;
				}

				G_node a = TAB_look(tempTab, def->head);

				for (Temp_tempList out = outhead; out; out = out->tail) {
					if (out->head == F_FP() || out->head == def->head) {
						continue;
					}

					G_node b = TAB_look(tempTab, out->head);
					if (!G_inNodeList(a, G_adj(b)) && !inTempList(use, out->head)) {
						G_addEdge(a, b);
						G_addEdge(b, a);
					}
				}

				for (Temp_tempList out = use; out; out = out->tail) {
					if (out->head == F_FP() || out->head == def->head) {
						continue;
					}

					G_node b = TAB_look(tempTab, out->head);

					if (!inMoveList(lg.moves, b, a)) {
						lg.moves = Live_MoveList(b, a, lg.moves);
					}
				}
			}
		}
	}

	return lg;
}

static Temp_tempList unionTempList(Temp_tempList a, Temp_tempList b) {
	Temp_tempList res = a;
	for (; b; b = b->tail) {
		if (!inTempList(a, b->head)) {
			res = Temp_TempList(b->head, res);
		}
	}

	return res;
}

static Temp_tempList subTempList(Temp_tempList a, Temp_tempList b) {
	Temp_tempList res = NULL;
	for (; a; a = a->tail) {
		if (!inTempList(b, a->head)) {
			res = Temp_TempList(a->head, res);
		}
	}

	return res;
}

static bool isEqual(Temp_tempList a, Temp_tempList b) {
	bool equal = TRUE;

	Temp_tempList p = a;
	for (; p; p = p->tail) {
		if (!inTempList(b, p->head)) {
			equal = FALSE;
			return equal;
		}
	}

	p = b;
	for (; p; p = p->tail) {
		if (!inTempList(a, p->head)) {
			equal = FALSE;
			return equal;
		}
	}

	return equal;
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