#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "util.h"
#include "absyn.h"
#include "temp.h"
#include "frame.h"

/* Lab5: your code below */

typedef struct Tr_exp_ *Tr_exp;

typedef struct Tr_expList_ *Tr_expList;

typedef struct Tr_access_ *Tr_access;

typedef struct Tr_accessList_ *Tr_accessList;

typedef struct Tr_level_ *Tr_level;

Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail);

Tr_access Tr_Access(Tr_level level, F_access access);

struct Tr_accessList_ {
	Tr_access head;
	Tr_accessList tail;	
};

Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail);

Tr_level Tr_outermost(void);

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);

Tr_accessList Tr_formals(Tr_level level);

Tr_access Tr_allocLocal(Tr_level level, bool escape);

Tr_exp Tr_Nil();
Tr_exp Tr_Int(int i);
Tr_exp Tr_String(string s);
Tr_exp Tr_Call(Tr_level callee, Temp_label label, Tr_expList params, Tr_level caller);
Tr_exp Tr_OpAlg(A_oper oper, Tr_exp left, Tr_exp right);
Tr_exp Tr_OpLog(A_oper oper, Tr_exp left, Tr_exp right, bool isStrCmp);
Tr_exp Tr_Record(int n, Tr_expList fields);
Tr_exp Tr_Seq(Tr_exp e1, Tr_exp e2);
Tr_exp Tr_Assign(Tr_exp var, Tr_exp exp);
Tr_exp Tr_IfThen(Tr_exp test, Tr_exp then);
Tr_exp Tr_IfThenElse(Tr_exp test, Tr_exp then, Tr_exp elsee);
Tr_exp Tr_While(Tr_exp test, Tr_exp body, Temp_label done);
Tr_exp Tr_For(Tr_access access, Tr_level level, Tr_exp lo, Tr_exp hi, Tr_exp body, Temp_label done);
Tr_exp Tr_Break(Temp_label done);
Tr_exp Tr_Array(Tr_exp size, Tr_exp init);
Tr_exp Tr_simpleVar(Tr_access access, Tr_level level);
Tr_exp Tr_fieldVar(Tr_exp addr, int n);
Tr_exp Tr_subscriptVar(Tr_exp addr, Tr_exp off);
void Tr_Func(Tr_exp body, Tr_level level);

T_stm Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals);
F_fragList Tr_getResult(void);

#endif
