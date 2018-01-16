#include "prog1.h"
#include <stdio.h>
#include <string.h>

/* struct & constructor from textbook */
typedef struct table *Table_;
struct table
{
	string id;
	int value;
	Table_ tail;
};

struct IntAndTable
{
	int i;
	Table_ t;
};

Table_ Table(string id, int value,
	Table_ tail) 
{
	Table_ t = checked_malloc(sizeof(*t));
	t->id = id;
	t->value = value;
	t->tail = tail;
	return t;
}

/* funtion prototype */
int maxargs_stm(A_stm stm);
int maxargs_exp(A_exp exp);
int maxargs_expList(A_expList expList);
int cal_printStmArgs(A_stm printStm);
int max(int left, int right);

Table_ interp_stm(A_stm stm, Table_ t);
struct IntAndTable interp_exp(A_exp exp, Table_ t);
int lookupValue(Table_ t, string key);
Table_ update(Table_ t, string key, int value);
int calculate(int left, A_binop oper, int right);

int maxargs(A_stm stm)
{
	/* 
	 * @return: how many args in printStm in stm
	 * @warn: remember that print statements can contain
	 *        expressions that contain other print statements
	 */

	return maxargs_stm(stm);
}

void interp(A_stm stm)
{
	/* TODO: interpret the straight-line program */
	if (!stm) {
		return;
	}

	interp_stm(stm, NULL);

	return;
}

int maxargs_stm(A_stm stm) 
{
	if (!stm) {
		return 0;
	}

	if (stm->kind == A_compoundStm) {
		/* handle CompoundStm */
		return max(maxargs_stm(stm->u.compound.stm1),
			maxargs_stm(stm->u.compound.stm2));
	} else if (stm->kind == A_assignStm) {
		/* handle AssignStm */
		return maxargs_exp(stm->u.assign.exp);
	} else {
		/* handle PrintStm */
		return max(cal_printStmArgs(stm),
			maxargs_expList(stm->u.print.exps));
	}

	return 0;
}

int maxargs_exp(A_exp exp)
{
	if (!exp) {
		return 0;
	}

	if (exp->kind == A_idExp || exp->kind == A_numExp) {
		/* handle IdExp and NumExp */
		return 0;
	} else if (exp->kind == A_opExp) {
		/* handle OpExp */
		return max(maxargs_exp(exp->u.op.left),
			maxargs_exp(exp->u.op.right));
	} else {
		/* handle EseqExp */
		return max(maxargs_stm(exp->u.eseq.stm),
			maxargs_exp(exp->u.eseq.exp));
	}

	return 0;
}

int maxargs_expList(A_expList expList)
{
	if (!expList) {
		return 0;
	}

	if (expList->kind == A_pairExpList) {
		/* handle PairExpList */
		return max(maxargs_exp(expList->u.pair.head), 
			maxargs_expList(expList->u.pair.tail));
	} else {
		/* handle LastExpList */
		return maxargs_exp(expList->u.last);
	}

	return 0;
}

int cal_printStmArgs(A_stm printStm)
{
	if (!printStm) {
		return 0;
	}

	A_expList expList = printStm->u.print.exps;
	if (!expList) {
		return 0;
	}

	int count = 1;
	while (expList->kind == A_pairExpList) {
		count++;
		expList = expList->u.pair.tail;
	}

	return count;
}

int max(int left, int right) 
{
	if (left >= right) {
		return left;
	}

	return right;  
}

Table_ interp_stm(A_stm stm, Table_ t)
{
	if (!stm) {
		return t;
	}

	if (stm->kind == A_compoundStm) {
		/* handle CompoundStm */
		t = interp_stm(stm->u.compound.stm1, t);
		t = interp_stm(stm->u.compound.stm2, t);
	} else if (stm->kind == A_assignStm) {
		/* handle AssignStm */
		struct IntAndTable res = interp_exp(stm->u.assign.exp, t);
		t = update(res.t, stm->u.assign.id, res.i);
	} else {
		/* handle PrintStm */
		A_expList expList = stm->u.print.exps;	
		int count = 0;
		while (expList->kind == A_pairExpList) {
			struct IntAndTable res = interp_exp(expList->u.pair.head, t);
			t = res.t;
			if (count == 0) {
				printf("%d", res.i);
			} else {
				printf(" %d", res.i);
			}

			count++;
			expList = expList->u.pair.tail;
		}

		/* LastExpList */
		struct IntAndTable res = interp_exp(expList->u.last, t);
		t = res.t;
		if (count == 0) {
			printf("%d\n", res.i);
		} else {
			printf(" %d\n", res.i);
		}
	}

	return t;
}

struct IntAndTable interp_exp(A_exp exp, Table_ t)
{
	struct IntAndTable res;

	if (!exp) {
		return res;
	}

	if (exp->kind == A_idExp) {
		/* handle IdExp */
		res.i = lookupValue(t, exp->u.id);
		res.t = t;
	} else if (exp->kind == A_numExp) {
		/* handle numExp */
		res.i = exp->u.num;
		res.t = t;
	} else if (exp->kind == A_opExp) {
		/* handle OpExp */
		struct IntAndTable res1 = interp_exp(exp->u.op.left, t);
		struct IntAndTable res2 = interp_exp(exp->u.op.right, res1.t);
		res.i = calculate(res1.i, exp->u.op.oper, res2.i);
		res.t = res2.t;
	} else {
		/* handle EseqExp */
		res.t = interp_stm(exp->u.eseq.stm, t);
		res = interp_exp(exp->u.eseq.exp, res.t);
	}

	return res;
}

int lookupValue(Table_ t, string key)
{
	while (t) {
		if (!strcmp(key, t->id)) {
			return t->value;
		}

		t = t->tail;
	}
}

Table_ update(Table_ t, string key, int value)
{
	/* 
	 * a naive way to update table, just
	 * put the new key-value in the head
	 */
	return Table(key, value, t);
}

int calculate(int left, A_binop oper, int right)
{
	switch(oper) {
		case A_plus:
			return left + right;

		case A_minus:
			return left - right;

		case A_times:
			return left * right;

		case A_div:
			return left / right;

		default:
			return 0;
	}
}

