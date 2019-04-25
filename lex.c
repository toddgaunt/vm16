/* See LICENSE file for copyright and license details */
#include <ctype.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "txt.h"
#include "lex.h"

/* Keywords defined by this lexer */
struct {char const *bytes; token_k kind;} kw[] = {
	/* Instructions */
	{"lui", TOK_LUI},   {"auipc", TOK_AUIPC}, {"jalr", TOK_JALR},
	{"beq", TOK_BEQ},   {"lw", TOK_LW},       {"sw", TOK_SW},
	{"addi", TOK_ADDI}, {"add", TOK_ADD},     {"sub", TOK_SUB},
	{"srl", TOK_SRL},   {"sll", TOK_SLL},     {"nand", TOK_NAND},
	{"and", TOK_AND},   {"or", TOK_OR},       {"lt", TOK_LT},
	/* Registers */
	{"zero", TOK_ZERO}, {"ra", TOK_RA}, {"sp", TOK_SP}, {"fp", TOK_FP},
	{"t0", TOK_T0},     {"t1", TOK_T1}, {"t2", TOK_T2}, {"t3", TOK_T3},
	/* Directives */
	{"nop", TOK_NOP}, {"halt", TOK_HALT}, {"la", TOK_LA}, {"li", TOK_LI},
	{"load", TOK_LOAD}, {"store", TOK_STORE}, {"jmp", TOK_JMP},
	{".word", TOK_WORD}, 
};

/* Supports both kinds of C-style comments */
static void
strip_whitespace_and_comments(struct txt *in)
{
	while (txt_at(in)[0]) {
		if (isspace(txt_at(in)[0])) {
			txt_get(in);
		} else if (txt_at(in)[0] == '/' && txt_at(in)[1] == '/') {
			while (txt_at(in)[0] && '\n' != txt_at(in)[0])
				txt_get(in);
		} else if (txt_at(in)[0] == '/' && txt_at(in)[1] == '*') {
			while (txt_at(in)[0] && !('*' == txt_at(in)[0] && '/' == txt_at(in)[1]))
				txt_get(in);
			txt_get(in);
			txt_get(in);
		} else {
			break;
		}
	}
}

static int 
isdelim(char ch)
{
	return isspace(ch) || '\0' == ch || ',' == ch;
}

static token_k
lex_raw_string(struct txt *in)
{
	while('`' != txt_at(in)[0]) {
		if (!txt_at(in)[0])
			return TOK_ERROR;
		txt_get(in);
	}
	txt_get(in);
	return TOK_RSTR;
}

static token_k
lex_nondecimal(struct txt *in)
{
	/* Hexadecimal numbers */
	if (txt_at(in)[0] == 'x') {
		do {
			txt_get(in);
		} while (isxdigit(txt_at(in)[0]));
		return TOK_HEX;
	}
	/* Octal numbers */
	while (isdigit(txt_at(in)[0]))
		txt_get(in);
	return TOK_OCT;
}

static token_k
lex_decimal(struct txt *in)
{
	while (isdigit(txt_at(in)[0]))
		txt_get(in);
	return TOK_DEC;
}

struct token
lex(struct txt *in)
{
	char ch;
	struct token rv;
	size_t i;

	strip_whitespace_and_comments(in);
	rv.row = in->row;
	rv.col = in->col;
	rv.bytes = txt_at(in);
	ch = txt_get(in);
	switch (ch) {
	case '\0':
		rv.kind = TOK_EOF;
		break;
	case ',':
		rv.kind = TOK_COMMA;
		break;
	case '-':
		rv.kind = TOK_DASH;
		break;
	case '`':
		rv.kind = lex_raw_string(in);
		break;
	case '0':
		rv.kind = lex_nondecimal(in);
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		rv.kind = lex_decimal(in);
		break;
	default:
		while (!isdelim(txt_at(in)[0]) && isalnum(txt_at(in)[0]))
			txt_get(in);
		rv.kind = TOK_IDENT;
		/* Check for special identifiers */
		for (i = 0; i < sizeof(kw)/sizeof(*kw); ++i) {
			if (txt_at(in) - rv.bytes == (long)strlen(kw[i].bytes)
			&& !strncmp(kw[i].bytes, rv.bytes, txt_at(in) - rv.bytes))
				rv.kind = kw[i].kind;
		}
	}
	rv.len = txt_at(in) - rv.bytes;
	return rv;
}
