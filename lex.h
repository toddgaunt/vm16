#ifndef LEX_H__
#define LEX_H__

#include "txt.h"

typedef enum {
	TOK_ERROR = -1,
	TOK_EOF = 0,
	TOK_COMMA,
	TOK_DASH,

	/* Instructions */
	TOK_LUI,
	TOK_AUIPC,
	TOK_JALR,
	TOK_BEQ,
	TOK_LW,
	TOK_SW,
	TOK_ADDI,
	TOK_ADD,
	TOK_SUB,
	TOK_SRL,
	TOK_SLL,
	TOK_NAND,
	TOK_AND,
	TOK_OR,
	TOK_LT,

	/* Registers */
	TOK_ZERO,
	TOK_RA,
	TOK_SP,
	TOK_FP,
	TOK_T0,
	TOK_T1,
	TOK_T2,
	TOK_T3,

	TOK_IDENT,
	TOK_RSTR,
	TOK_OCT,
	TOK_DEC,
	TOK_HEX,

	/* Directives */
	TOK_NOP,
	TOK_HALT,
	TOK_LA,
	TOK_LI,
	TOK_LOAD,
	TOK_STORE,
	TOK_JMP,
	TOK_WORD,
} token_k;

struct token {
	size_t row;
	size_t col;
	token_k kind;
	size_t len;
	char const *bytes;
};

/* Lex a token from a file */
struct token
lex(struct txt *in);

#endif
