#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "errormsg.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "codegen.h"
#include "table.h"

//Lab 6: put your code here
#define MAXLINE 40
#define caller_save L(F_EAX(), L(F_ECX(), L(F_EDX(), NULL)))
#define callee_save L(F_EBX(), L(F_ESI(), L(F_EDI(), NULL)))

static void emit(AS_instr inst);
static void munchStm(T_stm s);
static Temp_temp munchExp(T_exp e);
static void munchArgs(T_expList l);
Temp_tempList L(Temp_temp h, Temp_tempList t);

static AS_instrList iList = NULL, last = NULL;

static void emit(AS_instr inst) {
    if (last != NULL) {
    	last->tail = AS_InstrList(inst, NULL);
    	last = last->tail;
    } else {
    	iList = AS_InstrList(inst, NULL);
    	last = iList;
    }
}

AS_instrList F_codegen(F_frame f, T_stmList stmList) {
	AS_instrList list;
	T_stmList sl;

	/* save callee save reg */
	Temp_temp saveEbx = Temp_newtemp();
	Temp_temp saveEsi = Temp_newtemp();
	Temp_temp saveEdi = Temp_newtemp();
	emit(AS_Move("movl `s0, `d0\n", L(saveEbx, NULL), L(F_EBX(), NULL)));
	emit(AS_Move("movl `s0, `d0\n", L(saveEsi, NULL), L(F_ESI(), NULL)));
	emit(AS_Move("movl `s0, `d0\n", L(saveEdi, NULL), L(F_EDI(), NULL)));
	// emit(AS_Oper("pushl `s0\n", NULL, L(F_EBX(), NULL),AS_Targets(NULL)));
	// emit(AS_Oper("pushl `s0\n", NULL, L(F_ESI(), NULL),AS_Targets(NULL)));
	// emit(AS_Oper("pushl `s0\n", NULL, L(F_EDI(), NULL),AS_Targets(NULL)));
	// emit(AS_Oper("pushl `s0\n", NULL, L(F_ECX(), NULL),AS_Targets(NULL)));
	// emit(AS_Oper("pushl `s0\n", NULL, L(F_EDX(), NULL),AS_Targets(NULL)));

	for (sl = stmList; sl; sl = sl->tail) {
    	munchStm(sl->head);
	}
	//emit(AS_Oper("", NULL, L(F_EAX(), L(F_EBX(), L(F_ESI(), L(F_EDI(), NULL)))), AS_Targets(NULL), FALSE));
	/* restore callee save reg */
	emit(AS_Move("movl `s0, `d0\n", L(F_EBX(), NULL), L(saveEbx, NULL)));
	emit(AS_Move("movl `s0, `d0\n", L(F_ESI(), NULL), L(saveEsi, NULL)));
	emit(AS_Move("movl `s0, `d0\n", L(F_EDI(), NULL), L(saveEdi, NULL)));

	// emit(AS_Oper("popl `d0\n", L(F_EDX(), NULL), NULL,AS_Targets(NULL)));
	// emit(AS_Oper("popl `d0\n", L(F_ECX(), NULL), NULL,AS_Targets(NULL)));
	// emit(AS_Oper("popl `d0\n", L(F_EDI(), NULL), NULL,AS_Targets(NULL)));
	// emit(AS_Oper("popl `d0\n", L(F_ESI(), NULL), NULL,AS_Targets(NULL)));
	// emit(AS_Oper("popl `d0\n", L(F_EBX(), NULL), NULL,AS_Targets(NULL)));

	list = iList;
	iList = NULL;
	last = NULL;
	return list;
}

Temp_tempList L(Temp_temp h, Temp_tempList t) {
	return Temp_TempList(h, t);
}

static void munchStm(T_stm s) {
	/* T_LABEL */
	/* 
	 * label:
	 */
	if (s->kind == T_LABEL) {
    	char *inst = checked_malloc(MAXLINE * sizeof(char));

    	sprintf(inst, "%s:", Temp_labelstring(s->u.LABEL));
    	emit(AS_Label(inst, s->u.LABEL));
    	return;
	}

	/* T_JUMP */
	/*
	 * jmp label
	 */
	if (s->kind == T_JUMP) {
		char *inst = checked_malloc(MAXLINE * sizeof(char));
		Temp_label label = s->u.JUMP.exp->u.NAME;
		sprintf(inst, "jmp %s", Temp_labelstring(label));
		emit(AS_Oper(inst, NULL, NULL, AS_Targets(s->u.JUMP.jumps), TRUE));
		return;
	}

	/* T_CJUMP */
	/*
	 * cmp left, right
	 * cjump_op trueLabel
	 */
	if (s->kind == T_CJUMP) {	
		char *inst = checked_malloc(MAXLINE * sizeof(char));
		Temp_temp left = munchExp(s->u.CJUMP.left);
		Temp_temp right = munchExp(s->u.CJUMP.right);

		char *op;
		if (s->u.CJUMP.op == T_eq) {
			op = String("je");
		} else if (s->u.CJUMP.op == T_ne) {
			op = String("jne");
		} else if (s->u.CJUMP.op == T_lt) {
			op = String("jl");
		} else if (s->u.CJUMP.op == T_gt) {
			op = String("jg");
		} else if (s->u.CJUMP.op == T_le) {
			op = String("jle");
		} else if (s->u.CJUMP.op == T_ge) {
			op = String("jge");
		}

		emit(AS_Oper("cmp `s0, `s1", NULL, L(right, L(left, NULL)), AS_Targets(NULL), FALSE));
		sprintf(inst, "%s %s", op, Temp_labelstring(s->u.CJUMP.true));
		emit(AS_Oper(inst, NULL, NULL, AS_Targets(Temp_LabelList(s->u.CJUMP.true,NULL)), FALSE));
		return;
	}

	/* T_MOVE */
	if (s->kind == T_MOVE) {
		/*
		 * movl src, dst
		 */
		if (s->u.MOVE.dst->kind == T_TEMP) {
			Temp_temp src = munchExp(s->u.MOVE.src);
			Temp_temp dst = s->u.MOVE.dst->u.TEMP;

			emit(AS_Move("movl `s0, `d0", L(dst, NULL), L(src, NULL)));
			return;
		}

		/*
		 * movl src, mem
		 */
		if (s->u.MOVE.dst->kind == T_MEM) {
			Temp_temp src = munchExp(s->u.MOVE.src);
			Temp_temp mem = munchExp(s->u.MOVE.dst->u.MEM);

			emit(AS_Oper("movl `s0, (`s1)", NULL, L(src, L(mem, NULL)), AS_Targets(NULL), FALSE));
			return;
		}
	}

	/* T_EXP */
    /* emit nothing */
    if (s->kind == T_EXP) {
    	munchExp(s->u.EXP);
    	return;
	}
}

static void munchArgs(T_expList l) {
	if (l) {
    	Temp_temp s;
		munchArgs(l->tail);
    	s = munchExp(l->head);

    	emit(AS_Oper("pushl `s0", NULL, L(s, NULL), AS_Targets(NULL), FALSE));
	}
}

static Temp_temp munchExp(T_exp e) {

	/* T_BINOP */
	if (e->kind == T_BINOP) {
    	/*
    	 * movl left, dst
    	 * addl right, dst
    	 */
		if (e->u.BINOP.op == T_plus) {
			Temp_temp d = Temp_newtemp();
			Temp_temp left = munchExp(e->u.BINOP.left);
			Temp_temp right = munchExp(e->u.BINOP.right);

			emit(AS_Move("movl `s0, `d0", L(d, NULL), L(left, NULL)));
			emit(AS_Oper("addl `s0, `d0", L(d, NULL), 
				L(right, L(d, NULL)), AS_Targets(NULL), FALSE));
			return d;
		}

    	/*
    	 * movl left, dst
    	 * subl right, dst
    	 */
		if (e->u.BINOP.op == T_minus) {
			Temp_temp d = Temp_newtemp();
			Temp_temp left = munchExp(e->u.BINOP.left);
			Temp_temp right = munchExp(e->u.BINOP.right);

			emit(AS_Move("movl `s0, `d0", L(d, NULL), L(left, NULL)));
			emit(AS_Oper("subl `s0, `d0", L(d, NULL), 
				L(right, L(d, NULL)), AS_Targets(NULL), FALSE));
			return d;
		}

		/*
    	 * movl left, dst
    	 * imul right, dst
    	 */
		if (e->u.BINOP.op == T_mul) {
			Temp_temp d = Temp_newtemp();
			Temp_temp left = munchExp(e->u.BINOP.left);
			Temp_temp right = munchExp(e->u.BINOP.right);

			emit(AS_Move("movl `s0, `d0", L(d, NULL), L(left, NULL)));
			emit(AS_Oper("imul `s0, `d0", L(d, NULL), 
				L(right, L(d, NULL)), AS_Targets(NULL), FALSE));
			return d;
		}

		/* div is a little weird, since eax <- quotient edx <- remainder */
		/*
    	 * movl left, %eax
    	 * cltd #use eax's sign bit to fill edx
    	 * idivl right #now, q in eax and remainder in edx
    	 */
		if (e->u.BINOP.op == T_div) {
			Temp_temp d = Temp_newtemp();
			Temp_temp left = munchExp(e->u.BINOP.left);
			Temp_temp right = munchExp(e->u.BINOP.right);
		
			emit(AS_Move("movl `s0, `d0", L(F_EAX(), NULL), L(left, NULL)));
			emit(AS_Oper("cltd", L(F_EDX(), L(F_EAX(), NULL)), L(F_EAX(), NULL), AS_Targets(NULL), FALSE));
			emit(AS_Oper("idivl `s0", L(F_EDX(), L(F_EAX(), NULL)), 
				L(right, L(F_EDX(), L(F_EAX(), NULL))), AS_Targets(NULL), FALSE));
			emit(AS_Move("movl `s0, `d0", L(d, NULL), L(F_EAX(), NULL)));
			return d;
		}
    }

    /* T_MEM */
    /*
     * movl mem, dst
     */
	if (e->kind == T_MEM) {
		char *inst = checked_malloc(MAXLINE * sizeof(char));
		Temp_temp d = Temp_newtemp();
		Temp_temp addr = munchExp(e->u.MEM);

		sprintf(inst, "movl (`s0), `d0");
		emit(AS_Oper(inst, L(d, NULL), L(addr, NULL), AS_Targets(NULL), FALSE));
		return d;
	}

	/* T_TEMP */
	/* emit nothing */
	if (e->kind == T_TEMP) {
		return e->u.TEMP;
	}

	/* T_NAME */
	/*
	 * movl $str, dst
	 */
	if (e->kind == T_NAME) {
		char *inst = checked_malloc(MAXLINE * sizeof(char));
		Temp_temp d = Temp_newtemp();

		sprintf(inst, "movl $%s, `d0", Temp_labelstring(e->u.NAME));
		emit(AS_Oper(inst, L(d, NULL), NULL, AS_Targets(NULL), FALSE));
		return d;
	}

	/* T_CONST */
	/*
	 * movl $imm, dst
	 */
	if (e->kind == T_CONST) {
		char *inst = checked_malloc(MAXLINE * sizeof(char));
		Temp_temp d = Temp_newtemp();

		sprintf(inst, "movl $%d, `d0", e->u.CONST);
		emit(AS_Oper(inst, L(d, NULL), NULL, AS_Targets(NULL), FALSE));
		return d;
	}

	/* T_CALL */
	/* 
	 * pushl args_k
	 * push1 args_k-1....
	 * push1 args_0
	 * call func #put res into eax
	 * movl eax, dst
	 */
	if (e->kind == T_CALL) {
	    Temp_temp d = Temp_newtemp();
	    Temp_label func = e->u.CALL.fun->u.NAME;
	    char *inst = checked_malloc(MAXLINE * sizeof(char));

	    munchArgs(e->u.CALL.args);

	    sprintf(inst, "call %s", Temp_labelstring(func));
	    emit(AS_Oper(inst, caller_save, NULL, AS_Targets(NULL), FALSE));
	    emit(AS_Move("movl `s0, `d0", L(d, NULL), L(F_EAX(), NULL)));

	    return d;
	}
}