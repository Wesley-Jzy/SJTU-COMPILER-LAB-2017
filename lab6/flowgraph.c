#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "errormsg.h"
#include "table.h"

Temp_tempList FG_def(G_node n) {
	Temp_tempList dst = NULL;

	AS_instr instr = G_nodeInfo(n);

	if (instr->kind == I_OPER) {
		dst = instr->u.OPER.dst;
	} else if (instr->kind == I_LABEL) {
		dst = NULL;
	} else {
		dst = instr->u.MOVE.dst;
	}

	return dst;
}

Temp_tempList FG_use(G_node n) {
	Temp_tempList src = NULL;

	AS_instr instr = G_nodeInfo(n);

	if (instr->kind == I_OPER) {
		src = instr->u.OPER.src;
	} else if (instr->kind == I_LABEL) {
		src = NULL;
	} else {
		src = instr->u.MOVE.src;
	}

	return src;
}

bool FG_isMove(G_node n) {
	AS_instr instr = G_nodeInfo(n);
	return instr->kind == I_MOVE;
}

G_graph FG_AssemFlowGraph(AS_instrList il, F_frame f) {
	G_graph flowGraph = G_Graph();

	/* 
	 * Add all instr into graph,
	 * Add edge between none-jump instr
	 * Record all labels 
	 */
	TAB_table instrTab = TAB_empty();
	TAB_table labelTab = TAB_empty();

	AS_instrList cur;
	G_node now = NULL, last = NULL;
	for (cur = il; cur; cur = cur->tail) {
		now = G_Node(flowGraph, cur->head);
		TAB_enter(instrTab, cur->head, now);

		if (last) {
			AS_instr instr = G_nodeInfo(last);
			if (!(instr && instr->kind == I_OPER && instr->u.OPER.ucjmp)) {
				G_addEdge(last, now);
			}
		}

		if (cur->head->kind == I_LABEL) {
			TAB_enter(labelTab, cur->head->u.LABEL.label, now);
		}

		last = now;
	}

	now = NULL;
	G_node next = NULL;
	/* Add edge between jump instr */
	for (cur = il; cur; cur = cur->tail) {
		if (cur->head->kind == I_OPER) {
			now = TAB_look(instrTab, cur->head);

			Temp_labelList labels;
			for (labels = cur->head->u.OPER.jumps->labels; 
				labels; labels = labels->tail) {

				next = TAB_look(labelTab, labels->head);
				G_addEdge(now, next);
			}
		}
	}

	return flowGraph;
}
