#include <stdio.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "escape.h"
#include "table.h"
#include "helper.h"

static void traverseExp(S_table env, int depth, A_exp e);
static void traverseDec(S_table env, int depth, A_dec d);
static void traverseVar(S_table env, int depth, A_var v);

void Esc_findEscape(A_exp exp) {
	S_table escBaseEnv = S_empty();
    traverseExp(escBaseEnv, 0, exp);
}

escapeEntry EscapeEntry(int depth, bool *escape)
{
    escapeEntry entry = checked_malloc(sizeof(escapeEntry));
    entry->depth = depth;
    entry->escape = escape;
    return entry;
}

static void traverseExp(S_table env, int depth, A_exp e) {
	switch (e->kind) {
		case A_varExp: {
			traverseVar(env, depth, e->u.var);
			return;
		}

		case A_nilExp:
			return;

		case A_intExp:
			return;

		case A_stringExp:
			return;

		case A_callExp: {
			A_expList args;
			for (args = get_callexp_args(e); args; args = args->tail) {
				traverseExp(env, depth, args->head);
			}
			return;
		}

		case A_opExp: {	
			traverseExp(env, depth, e->u.op.left);
			traverseExp(env, depth, e->u.op.right);
			return;
		}

		case A_recordExp: {
			A_efieldList efields;
			for (efields = get_recordexp_fields(e); efields; efields = efields->tail) {
				traverseExp(env, depth, efields->head->exp);
			}
			return;
		}

		case A_seqExp: {
			A_expList exps;
			for (exps = get_seqexp_seq(e); exps; exps = exps->tail) {
				traverseExp(env, depth, exps->head);
			}
			return;
		}

		case A_assignExp: {
			A_var var = get_assexp_var(e);
			A_exp exp = get_assexp_exp(e);
			traverseVar(env, depth, var);
			traverseExp(env, depth, exp);
			return;
		}

		case A_ifExp: {
			A_exp test = get_ifexp_test(e);
			A_exp then = get_ifexp_then(e);
			A_exp elsee = get_ifexp_else(e);
			traverseExp(env, depth, test);
			traverseExp(env, depth, then);
			if (elsee) {
				traverseExp(env, depth, elsee);
			}
			return;
		}

		case A_whileExp: {
			A_exp test = get_whileexp_test(e);
			A_exp body = get_whileexp_body(e);
			traverseExp(env, depth, test);
			traverseExp(env, depth, body);
			return;
		}

		case A_forExp: {
			A_exp lo = get_forexp_lo(e);
			A_exp hi = get_forexp_hi(e);
			A_exp body = get_forexp_body(e);
			traverseExp(env, depth, lo);
			traverseExp(env, depth, hi);
			S_beginScope(env);
			e->u.forr.escape = FALSE;
			S_enter(env, get_forexp_var(e), EscapeEntry(depth, &(e->u.forr.escape)));
			traverseExp(env, depth, body);
			S_endScope(env);
			return;
		}

		case A_breakExp: {
			return;
		}

		case A_letExp: {
			A_decList decs;
			A_exp body = get_letexp_body(e);
			S_beginScope(env);
			for (decs = get_letexp_decs(e); decs; decs = decs->tail) {
				traverseDec(env, depth, decs->head);
			}
			traverseExp(env, depth, body);
			S_endScope(env);
			return;
		}

		case A_arrayExp: {
			A_exp size = get_arrayexp_size(e);
			A_exp init = get_arrayexp_init(e);
			traverseExp(env, depth, size);
			traverseExp(env, depth, init);
			return;
		}
	}
}

static void traverseDec(S_table env, int depth, A_dec d) {
	switch(d->kind) {
		case A_functionDec: {
			A_fundecList functions;
			A_fieldList params;
			A_fundec func;
			for (functions = get_funcdec_list(d); functions; functions = functions->tail) {
				func = functions->head;
				S_beginScope(env);
				for (params = func->params; params; params = params->tail) {
					params->head->escape = FALSE;
					S_enter(env, params->head->name, EscapeEntry(depth + 1, &params->head->escape));
				}
				traverseExp(env, depth + 1, func->body);
				S_endScope(env);
			}
			return;
		}

		case A_varDec: {
			A_exp init = get_vardec_init(d);
			S_symbol var = get_vardec_var(d);

			traverseExp(env, depth, init);
			d->u.var.escape = FALSE;
			S_enter(env, var, EscapeEntry(depth, &d->u.var.escape));
			return;
		}

		case A_typeDec: {
			return;
		}
	}
}

static void traverseVar(S_table env, int depth, A_var v) {
	switch (v->kind) {
		case A_simpleVar: {
			S_symbol id = get_simplevar_sym(v);
			escapeEntry entry = S_look(env, id);
			if (entry->depth < depth) {
				*(entry->escape) = TRUE;
			}
			return;
		}

		case A_fieldVar: {
			A_var lvalue = get_fieldvar_var(v);
			traverseVar(env, depth, lvalue);
			return;
		}

		case A_subscriptVar: {
			A_var lvalue = get_subvar_var(v);
			A_exp index = get_subvar_exp(v);
			traverseVar(env, depth, lvalue);
			traverseExp(env, depth, index);
			return;
		}
	}
}