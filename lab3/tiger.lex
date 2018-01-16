%{
/* Lab2 Attention: You are only allowed to add code in this file and start at Line 26.*/
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "errormsg.h"
#include "absyn.h"
#include "y.tab.h"

int charPos=1;

int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

/*
* Please don't modify the lines above.
* You can add C declarations of your own below.
*/

/* @function: getstr
 * @input: a string literal
 * @output: the string value for the input which has all the escape sequences 
 * translated into their meaning.
 */

/* counter for nested comments */
#define MAXSTR 2048
int comments = 0;
int len = 0;
char res[MAXSTR];

// char *getstr(const char *str)
// {
// 	if (!str) {
// 		return NULL;
// 	}

// 	char res[MAXSTR];
// 	/* ignore first " */
// 	unsigned int p = 1, len = 0;
// 	/* ignore last " */
// 	while (*(str + p) != '\"') {
// 		/* handle \ */
// 		if (!strncmp(str + p, "\\", 1)) {
// 			/* handle \n */
// 			if (!strncmp(str + p, "\\n", 2)) {
// 				res[len] = '\n';
// 				len++;
// 				p += 2;
// 			} 
// 			/* handle \t */
// 			else if (!strncmp(str + p, "\\t", 2)) {
// 				res[len] = '\t';
// 				len++;
// 				p += 2;
// 			}
// 			/* handle \\ */
// 			else if (!strncmp(str + p, "\\\\", 2)) {
// 				res[len] = '\\';
// 				len++;
// 				p += 2;
// 			}
// 			/* handle \" */
// 			else if (!strncmp(str + p, "\\\"", 2)) {
// 				res[len] = '"';
// 				len++;
// 				p += 2;
// 			}
// 			/* handle \ddd */
// 			else if (!strncmp(str + p, "\\\"", 2)) {
// 				res[len] = '"';
// 				len++;
// 				p += 2;
// 			}
// 			/* handle error \ */
// 			else {
// 				EM_error(EM_tokPos, "wrong use to \\");
// 				return NULL;
// 			}
// 		}
// 		/* handle real \n */
// 		else if (!strncmp(str + p, "\n", 1)) {
// 			EM_newline();
// 			res[len] = '\n';
// 			len++;
// 			p++;
// 		}

// 		/* handle other char */
// 		else {
// 			res[len] = *(str + p);
// 			len++;
// 			p++;
// 		}
// 	}
// 	/* add \0 */
// 	res[len] = '\0';

// 	/* just for pass test */
// 	if (len == 0) {
// 		strcpy(res, "(null)");
// 	}
	
// 	return String(res);
// }

%}
  /* You can add lex definitions here. */
space " "|"\t"
letter [a-zA-Z]
digit [0-9]
id {letter}({letter}|{digit}|_)*

%Start COMMENT ERROR STR
%%
	/* 
	 * Below is an example, which you can wipe out
	 * and write reguler expressions and actions of your own.
	 */ 

	/* comment */
	/* Warning: comment not begin with "(*",but "/*" ,wtf */
<INITIAL>"/*" {
	adjust(); 
	if (comments == 0) {
		BEGIN COMMENT;
	}
	comments++;
}
<COMMENT>"/*" {
	adjust();
	comments++;
}
<COMMENT>"*/" {
	adjust();
	comments--;
	if (comments == 0) {
		BEGIN INITIAL;
	}
}
<COMMENT>. {adjust();}
<COMMENT>"\n" {adjust(); EM_newline(); continue;}
<COMMENT><<EOF>> {EM_error(EM_tokPos, "unclosed comments"); BEGIN ERROR;}

	/* error handler */
<ERROR>.|"\n" {BEGIN INITIAL; yyless(0);}

	/* reserved words */
<INITIAL>"array" {adjust(); return ARRAY;}
<INITIAL>"if" {adjust(); return IF;}
<INITIAL>"then" {adjust(); return THEN;}
<INITIAL>"else" {adjust(); return ELSE;}
<INITIAL>"while" {adjust(); return WHILE;}
<INITIAL>"for" {adjust(); return FOR;}
<INITIAL>"to" {adjust(); return TO;}
<INITIAL>"do" {adjust(); return DO;}
<INITIAL>"let" {adjust(); return LET;}
<INITIAL>"in" {adjust(); return IN;}
<INITIAL>"end" {adjust(); return END;}
<INITIAL>"of" {adjust(); return OF;}
<INITIAL>"break" {adjust(); return BREAK;}
<INITIAL>"nil" {adjust(); return NIL;}
<INITIAL>"function" {adjust(); return FUNCTION;}
<INITIAL>"var" {adjust(); return VAR;}
<INITIAL>"type" {adjust(); return TYPE;}

	/* id */
<INITIAL>{id} {adjust(); yylval.sval = String(yytext); return ID;}

	/* str */
	/* begin str */
	/* <INITIAL>\"\" {adjust(); yylval.sval = String("(null)"); return STRING;} */
<INITIAL>\" {adjust(); len = 0; BEGIN STR;}
	/* handle /n */
<STR>"\\n" {charPos += yyleng; res[len] = '\n'; len++;}
	/* handle /t */
<STR>"\\t" {charPos += yyleng; res[len] = '\t'; len++;}
	/* handle /^c, what's this?? */

	/* handle \ddd */
<STR>\\{digit}{1,3} {charPos += yyleng; res[len] = atoi(yytext+1); len++;}
	/* handle \" */
<STR>\\\" {charPos += yyleng; res[len] = '"'; len++;}
	/* handle \\ */
<STR>\\\\ {charPos += yyleng; res[len] = '\\'; len++;}
	/* handle \f...f\ */
<STR>\\[ \t\n\f]+\\ {charPos += yyleng;}
	/* wrong \ */
<STR>\\ {adjust(); EM_error(EM_tokPos, "wrong use on \\"); BEGIN ERROR;}
	/* end str */
<STR>\" {
	charPos += yyleng; 
	res[len] = '\0'; 
	yylval.sval = String(res); 
	BEGIN INITIAL;
	return STRING;
}
	/* handle other char */
<STR>. {charPos += yyleng; strcpy(res + len, yytext); len += yyleng;}
	/* handle real \n */
<STR>"\n" {charPos += yyleng; strcpy(res + len, yytext); len += yyleng;}

	/* int */
<INITIAL>{digit}+ {adjust(); yylval.ival = atoi(yytext); return INT;}

	/* punctuation symbols */
<INITIAL>"," {adjust(); return COMMA;}
<INITIAL>":" {adjust(); return COLON;}
<INITIAL>";" {adjust(); return SEMICOLON;}
<INITIAL>"(" {adjust(); return LPAREN;}
<INITIAL>")" {adjust(); return RPAREN;}
<INITIAL>"[" {adjust(); return LBRACK;}
<INITIAL>"]" {adjust(); return RBRACK;}
<INITIAL>"{" {adjust(); return LBRACE;}
<INITIAL>"}" {adjust(); return RBRACE;}
<INITIAL>"." {adjust(); return DOT;}
<INITIAL>"+" {adjust(); return PLUS;}
<INITIAL>"-" {adjust(); return MINUS;}
<INITIAL>"*" {adjust(); return TIMES;}
<INITIAL>"/" {adjust(); return DIVIDE;}
<INITIAL>"=" {adjust(); return EQ;}
<INITIAL>"<>" {adjust(); return NEQ;}
<INITIAL>"<" {adjust(); return LT;}
<INITIAL>"<=" {adjust(); return LE;}
<INITIAL>">" {adjust(); return GT;}
<INITIAL>">=" {adjust(); return GE;}
<INITIAL>"&" {adjust(); return AND;}
<INITIAL>"|" {adjust(); return OR;}
<INITIAL>":=" {adjust(); return ASSIGN;}

	/* ignored tokens */
<INITIAL>{space} {adjust(); continue;}
<INITIAL>"\n" {adjust(); EM_newline(); continue;}

	/* cannot fit any one, it's illegal */
<INITIAL>. {adjust(); EM_error(EM_tokPos, "illegal character"); BEGIN ERROR;}

