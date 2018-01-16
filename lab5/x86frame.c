#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"
#include "tree.h"
#include "frame.h"

/*Lab5: Your implementation here.*/
const int F_wordSize = 4;

//varibales
struct F_frame_ {
	F_accessList formals;
	Temp_label name;
	int size;
};

struct F_access_ {
	enum {inFrame, inReg} kind;
	union {
		int offset; //inFrame
		Temp_temp reg; //inReg
	} u;
};

F_accessList F_AccessList(F_access head, F_accessList tail) {
	F_accessList accesses = checked_malloc(sizeof(*accesses));
	accesses->head = head;
	accesses->tail = tail;
	return accesses;
}

static F_access InFrame(int offset) {
	F_access access = checked_malloc(sizeof(*access));
	access->kind = inFrame;
	access->u.offset = offset;
	return access;
}

static F_access InReg(Temp_temp reg) {
	F_access access = checked_malloc(sizeof(*access));
	access->kind = inReg;
	access->u.reg = reg;
	return access;
}

static F_accessList Formals(U_boolList formals, int offset) {
	if (formals) {
		return F_AccessList(InFrame(offset), Formals(formals->tail ,offset + F_wordSize));
	} else {
		return NULL;
	}
}

F_frame F_newFrame(Temp_label name, U_boolList formals) {
	F_frame frame = checked_malloc(sizeof(*frame));
	frame->formals = Formals(formals, F_wordSize);
	frame->name = name;
	frame->size = 0;
	return frame;
}

Temp_label F_name(F_frame f) {
	return f->name;
}

F_accessList F_formals(F_frame f) {
	return f->formals;
}

F_access F_allocLocal(F_frame f, bool escape) {
	if (escape) {
		f->size++;
		int offset = -(f->size * F_wordSize);
		return InFrame(offset);
	} else {
		return InReg(Temp_newtemp());
	}
}

/* ebp */
Temp_temp F_FP(void) {
	return Temp_newtemp();
}

/* eax, also for func return */
Temp_temp F_RV(void) {
	return Temp_newtemp();
}

T_exp F_Exp(F_access access, T_exp fp) {
	if (access->kind == inFrame) {
		return T_Mem(T_Binop(T_plus, fp, T_Const(access->u.offset)));
	} else {
		return T_Temp(access->u.reg);
	}
}

F_frag F_StringFrag(Temp_label label, string str) {  
	F_frag stringFrag = checked_malloc(sizeof(*stringFrag));
	stringFrag->kind = F_stringFrag;
	stringFrag->u.stringg.label = label;
	stringFrag->u.stringg.str = str;
	return stringFrag;
}                                                     
                                                      
F_frag F_ProcFrag(T_stm body, F_frame frame) {        
	F_frag procFrag = checked_malloc(sizeof(*procFrag));
	procFrag->kind = F_procFrag;
	procFrag->u.proc.body = body;
	procFrag->u.proc.frame = frame;
	return procFrag;
}                                                    
                                                      
F_fragList F_FragList(F_frag head, F_fragList tail) { 
	F_fragList fragList = checked_malloc(sizeof(*fragList));
	fragList->head = head;
	fragList->tail = tail;
	return fragList;                 
}      

T_exp F_externalCall(string s, T_expList args) {
	return T_Call(T_Name(Temp_namedlabel(s)), args);
}                                               

