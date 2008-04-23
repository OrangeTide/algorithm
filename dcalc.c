/* contributed by demoncrat@efnet.#c on march 2001. */
/* edited by megaton. added double precision numbers */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "dcalc.h"


static const char *complaint = NULL;

static void complain (const char *msg)
{
	if (complaint == NULL)
		complaint = msg;
}


enum {
	HALT, PUSH, ADD, SUB, MUL, DIV, MOD, POWER
};

typedef Value instruc;

static instruc code[CODESIZE];
static instruc *here = code;

static void gen (instruc opcode)
{
	if (code + CODESIZE <= here)
		{
			complain ("Expression too complicated");
			return;
		}

	*here++ = opcode;
}

static void gen_push (Value v)
{
	gen (PUSH);
	gen ((instruc) v);
}

static Value run()
{
	Value stack[STACKSIZE];
	Value *sp = stack + STACKSIZE;
	instruc *pc = code;

#define need(n)													\
	do {																	\
		if (sp - (n) < stack)								\
			{																	\
				complain ("Stack overflow");		\
				return 0;												\
			}																	\
	} while (0)

	for (;;)
		switch ((unsigned)*pc++)
			{
			case HALT: return sp[0]; break;
			case PUSH: need (1); *--sp = (Value) *pc++; break;
			case ADD: sp[1] += sp[0]; ++sp; break;
			case SUB: sp[1] -= sp[0]; ++sp; break;
			case MUL: sp[1] *= sp[0]; ++sp; break;
			case DIV: sp[1] /= sp[0]; ++sp; break;
			case MOD: sp[1] = fmod( sp[1], sp[0]); ++sp; break;
			case POWER: sp[1] = pow (sp[1], sp[0]); ++sp; break;
			default: assert (0);
			}
}


static const char *p;
static int token;
static Value token_value;

static void next () 
{
	char *endptr;

	switch (*p)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			token = PUSH;
			token_value = strtod (p, &endptr);
			p = endptr;
			break;

		case '+': 
		case '-': 
		case '*': 
		case '/': 
		case '%': 
		case '^':
		case '(':
		case ')':
			token = *p++;
			break;

		case ' ':
		case '\t':
			++p;
			next ();
			break;

		case '\0':
			token = '\0';
			break;

		default:
			complain ("Syntax error: unknown token type");
			token = '\0';
			break;
		}
}

static void parse_expr (int precedence);

static void parse_factor () 
{
	switch (token)
		{
		case PUSH:
			gen_push (token_value);
			next ();
			break;

		case '-':			/* unary minus */
			next ();
			gen_push (0);
			parse_factor ();
			gen (SUB);
			break;

		case '(':
			next ();
			parse_expr (0);
			if (token != ')')
	complain ("Syntax error: expected ')'");
			next ();
			break;

		default:
			complain ("Syntax error: expected a factor");
			next ();
		}
}

static void parse_expr (int precedence) 
{
	parse_factor ();
	for (;;) 
		{
			int l, r, rator;	 /* left precedence, right precedence, and operator */

			switch (token) {
			case '+': l = 1; r = 2; rator = ADD; break;
			case '-': l = 1; r = 2; rator = SUB; break;
	
			case '*': l = 3; r = 4; rator = MUL; break;
			case '/': l = 3; r = 4; rator = DIV; break;
			case '%': l = 3; r = 4; rator = MOD; break;

			case '^': l = 5; r = 5; rator = POWER; break;
	
			default: return;
			}

			if (l < precedence)
	return;

			next ();
			parse_expr (r);
			gen (rator);
		}
}

const char *dcalc (Value *result, const char *expression)
{
	*result = 0;

	complaint = NULL;
	p = expression;
	here = code;
	next ();
	parse_expr (0);
	if (*p != '\0')
		complain ("Syntax error: unexpected token");

	if (!complaint)
		{
			gen (HALT);
			*result = run ();
		}

	return complaint;
}

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
