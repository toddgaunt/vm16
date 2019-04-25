/* See LICENSE file for copyright and license details */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "log.h"
#include "symtab.h"
#include "txt.h"
#include "vm16.h"

int pass = 0;
uint16_t pc = VM16_ADDR_START;
size_t idx = 0;
struct symtab *symtab;
struct token next;

/* Print out an assembler error message */
static void
err(struct txt *in, struct token const *t, char const *msg)
{
	size_t i;
	fprintf(stderr, "--> %s:%zu:%zu\n", in->name, t->row, t->col);
	char const *line = t->bytes - t->col + 1;

	for (i = 0; line[i] != '\n' && line[i] != '\0'; ++i)
		putc(line[i], stderr);
	putc('\n', stderr);
	if (!msg)
		return;
	for (i = 0; i < t->col - 1; ++i)
		putc(' ', stderr);
	for (i = 0; i < (t->len ? t->len : 1); ++i)
		putc('^', stderr);
	fprintf(stderr, " %s\n", msg);
}

/* Generate asmd instructions to file */
static void
gen(uint16_t *out, uint16_t instr)
{
	if (pass == 1)
		out[idx++] = instr;
	pc += 1;
}

/* Get the next token */
static void
get(struct txt *in) {
	next = lex(in);
}

/* Parse a comma else error */
static bool
parse_comma(struct txt *in)
{
	get(in);
	if (next.kind != TOK_COMMA) {
		err(in, &next, "expected comma");
		exit(-1);
	}
	return true;
}

/* Parse a register name and write the register number to `r` else error */
static bool
parse_reg(struct txt *in, uint16_t *r)
{
	get(in);
	if (next.kind < TOK_ZERO || next.kind > TOK_T3) {
		err(in, &next, "expected register");
		exit(-1);
	}
	*r = next.kind - TOK_ZERO;
	return true;
}

/* Parse a octal/decimal/hexadecimal number and write it to `n` else error */
static bool
parse_number(struct txt *in, uint8_t maxbit, uint16_t *n)
{
	uint64_t tmp;
	bool negate = false;

top:
	get(in);
	switch (next.kind) {
	case TOK_OCT:
	case TOK_DEC:
	case TOK_HEX:
		break;
	case TOK_DASH:
		negate = !negate;
		goto top;
	default:
		err(in, &next, "expected integer literal");
		exit(-1);
	}

	tmp = strtol(next.bytes, NULL, 0);
	if (tmp > (uint64_t)(1 << maxbit) - 1) {
		err(in, &next, "integer literal too large");
		exit(-1);
	}
	if (negate)
		tmp = -tmp;
	*n = tmp;
	return true;
}

static bool
parse_label(struct txt *in, uint16_t *addr)
{
	uint16_t *tmp;

	get(in);
	if (next.kind != TOK_IDENT) {
		err(in, &next, "expected label");
		exit(-1);
	}

	if (pass == 0) {
		*addr = 0;
		return true;
	}

	tmp = symtab_at(symtab, &next);
	if (!tmp) {
		err(in, &next, "use of undefined label");
		exit(-1);
	}
	*addr = *tmp;
	return true;
}

static void
asm_ori(struct txt *in, uint16_t  *out, uint16_t opcode)
{
	uint16_t rd = 0, im10 = 0;

        parse_reg(in, &rd);
        parse_comma(in);
        parse_number(in, 10, &im10);
	gen(out, vm16_ori(opcode, rd, im10));
}

static void
asm_orri(struct txt *in, uint16_t *out, uint16_t opcode)
{
	uint16_t rd = 0, r1 = 0, im7 = 0;

        parse_reg(in, &rd);
        parse_comma(in);
        parse_reg(in, &r1);
        parse_comma(in);
        parse_number(in, 7, &im7);
	gen(out, vm16_orri(opcode, rd, r1, im7));
}

static void
asm_math(struct txt *in, uint16_t  *out, int alt)
{
	uint16_t rd = 0, r1 = 0, r2 = 0;

        parse_reg(in, &rd);
        parse_comma(in);
        parse_reg(in, &r1);
        parse_comma(in);
        parse_reg(in, &r2);
	gen(out, vm16_orrar(VM16_MATH, rd, r1, alt, r2));
}

static void
asm_word(struct txt *in, uint16_t *out)
{
	uint16_t word;
	parse_number(in, 16, &word);
	gen(out, word);
}

static void
asm_li(struct txt *in, uint16_t *out)
{
	uint16_t rd = 0, im16 = 0;

        parse_reg(in, &rd);
        parse_comma(in);
        parse_number(in, 16, &im16);

	gen(out, vm16_ori(VM16_LUI, rd, (im16 & 0xFFC0) >> 6));
	gen(out, vm16_orri(VM16_ADDI, rd, rd, im16 & 0x3F));
}

size_t
assemble(struct txt *in, uint16_t out[VM16_MM_SIZE])
{
	symtab = symtab_create(1024);
	for (pass = 0; pass < 2; ++pass) {
		pc = VM16_ADDR_START;
		get(in);
		while (next.kind != TOK_EOF) {
			if (next.kind == TOK_IDENT) {
				if (pass == 0) {
					struct token const *prev;

					prev = symtab_getk(symtab, &next);
					if (prev) {
						err(in, &next, "duplicate label");
						err(in, prev, "previously defined here");
						exit(-1);
					}
					*symtab_getv(symtab, &next) = pc;
				}
				get(in);
			}

			switch (next.kind) {
			case TOK_LUI:
				asm_ori(in, out, VM16_LUI);
				break;
			case TOK_AUIPC:
				asm_ori(in, out, VM16_AUIPC);
				break;
			case TOK_JALR:
				asm_orri(in, out, VM16_JALR);
				break;
			case TOK_BEQ:
				asm_orri(in, out, VM16_BEQ);
				break;
			case TOK_LW:
				asm_orri(in, out, VM16_LW);
				break;
			case TOK_SW:
				asm_orri(in, out, VM16_SW);
				break;
			case TOK_ADDI:
				asm_orri(in, out, VM16_ADDI);
				break;
			case TOK_ADD:
				asm_math(in, out, VM16_ADD);
				break;
			case TOK_SUB:
				asm_math(in, out, VM16_SUB);
				break;
			case TOK_SLL:
				asm_math(in, out, VM16_SLL);
				break;
			case TOK_SRL:
				asm_math(in, out, VM16_SRL);
				break;
			case TOK_NAND:
				asm_math(in, out, VM16_NAND);
				break;
			case TOK_AND:
				asm_math(in, out, VM16_AND);
				break;
			case TOK_OR:
				asm_math(in, out, VM16_OR);
				break;
			case TOK_LT:
				asm_math(in, out, VM16_LT);
				break;
			/* Directives */
			case TOK_NOP:
				gen(out, vm16_orri(VM16_ADDI, 0, 0, 0));
				break;
			case TOK_HALT:
				gen(out, vm16_orri(VM16_JALR, 0, 0, 0));
				break;
			case TOK_WORD:
				asm_word(in, out);
				break;
			case TOK_LI:
				asm_li(in, out);
				break;
			default:
				err(in, &next, "expected instruction");
				break;
			}
			get(in);
		}
		txt_reset(in);
	}
	return idx;
}
