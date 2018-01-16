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
#define MAXINS 100

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
	frame->formals = Formals(formals, 8);
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

int F_size(F_frame f) {
	return f->size * F_wordSize;
}

char *F_prolog(F_frame f) {
	char *out = checked_malloc(MAXINS * sizeof(char));
	sprintf(out, "pushl %%ebp\nmovl %%esp, %%ebp\nsubl $%d, %%esp\n", F_size(f));
	return out;
}

char *F_epilog(F_frame f) {
	char *out = checked_malloc(MAXINS * sizeof(char));
	sprintf(out, "leave\nret\n");
	return out;
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

int F_spill(F_frame f) {
	f->size++;
	int offset = -(f->size * F_wordSize);
	return offset;
}

static Temp_temp ebp = NULL;
static Temp_temp eax = NULL;
static Temp_temp ebx = NULL;
static Temp_temp ecx = NULL;
static Temp_temp edx = NULL;
static Temp_temp esi = NULL;
static Temp_temp edi = NULL;

Temp_temp F_FP(void) {
	if (ebp) {
		return ebp;
	} else {
		ebp = Temp_newtemp();
		return ebp;
	}
}

Temp_temp F_EAX(void) {
	if (eax) {
		return eax;
	} else {
		eax = Temp_newtemp();
		return eax;
	}
}

Temp_temp F_EBX(void) {
	if (ebx) {
		return ebx;
	} else {
		ebx = Temp_newtemp();
		return ebx;
	}
}

Temp_temp F_ECX(void) {
	if (ecx) {
		return ecx;
	} else {
		ecx = Temp_newtemp();
		return ecx;
	}
}

Temp_temp F_EDX(void) {
	if (edx) {
		return edx;
	} else {
		edx = Temp_newtemp();
		return edx;
	}
}

Temp_temp F_ESI(void) {
	if (esi) {
		return esi;
	} else {
		esi = Temp_newtemp();
		return esi;
	}
}

Temp_temp F_EDI(void) {
	if (edi) {
		return edi;
	} else {
		edi = Temp_newtemp();
		return edi;
	}
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

