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
uint16_t pc = 0;
struct symtab *symtab;
struct token next;

/* Print out an assembler error message */
static void
err(struct txt *in, struct token const *t, char const *msg)
{
	size_t i;
	fprintf(stderr, "--> %s:%zu:%zu\n", in->name, t->row, t->col);
	char const *line = t->str - t->col + 1;

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
	if (pass == 1) {
		//putc((instr & 0xFF00) >> 8, out);
		//putc(instr & 0x00FF, out);
		printf("%d", (instr & 0xFF00) >> 8);
		printf("%d\n", instr & 0x00FF);
	}
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
parse_num(struct txt *in, int maxbit, uint16_t *n)
{
	uint16_t tmp;
	size_t max;

	get(in);
	switch (next.kind) {
	case TOK_OCT: max = 4; break;
	case TOK_DEC: max = 4; break;
	case TOK_HEX: max = 3; break;
	default:
		err(in, &next, "expected number literal");
		exit(-1);
	}
	tmp = strtol(next.str, NULL, 0);
	if (next.len > max || tmp > (1 << maxbit) - 1) {
		err(in, &next, "decimal number exceeds 10 bit max");
		exit(-1);
	}
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
asm_lui(struct txt *in, uint16_t  *out)
{
	uint16_t rd = 0, im10 = 0;

        parse_reg(in, &rd);
        parse_comma(in);
        parse_num(in, 10, &im10);

	gen(out, vm16_ori(VM16_LUI, rd, im10));
}

static void
asm_auipc(struct txt *in, uint16_t  *out)
{
	uint16_t rd = 0, im10 = 0;

        parse_reg(in, &rd);
        parse_comma(in);
        parse_num(in, 10, &im10);
	gen(out, vm16_ori(VM16_AUIPC, rd, im10));
}

static void
asm_jalr(struct txt *in, uint16_t  *out)
{
	uint16_t rd = 0, addr = 0;
	uint16_t raddr; /* Relative address */

        parse_reg(in, &rd);
        parse_comma(in);
	parse_label(in, &addr);

	raddr = addr - pc - 3;
	gen(out, vm16_ori(VM16_AUIPC, rd, (addr & 0xFF80) >> 6));
	gen(out, vm16_orri(VM16_JALR, rd, rd, raddr & 0x007F));
}

static void
asm_beq(struct txt *in, uint16_t  *out)
{
	uint16_t rd = 0, r1 = 0, im7 = 0;

        parse_reg(in, &rd);
        parse_comma(in);
        parse_reg(in, &r1);
        parse_comma(in);
        parse_num(in, 10, &im7);
	gen(out, vm16_orri(VM16_BEQ, rd, r1, im7));
}

static void
asm_lw(struct txt *in, uint16_t  *out)
{
	uint16_t rd = 0, r1 = 0, addr = 0;
	uint16_t raddr; /* Relative address */

        parse_reg(in, &rd);
        parse_comma(in);
        parse_reg(in, &r1);
        parse_comma(in);
	parse_label(in, &addr);

	raddr = addr - pc;
	gen(out, vm16_ori(VM16_AUIPC, r1, raddr & 0xFFC0));
	gen(out, vm16_orri(VM16_LW, rd, r1, raddr & 0x003F));
}

static void
asm_sw(struct txt *in, uint16_t  *out)
{
	uint16_t rd = 0, addr = 0;
	uint16_t raddr; /* Relative address */

        parse_reg(in, &rd);
        parse_comma(in);
	parse_label(in, &addr);

	raddr = addr - pc;
	gen(out, vm16_ori(VM16_AUIPC, rd, raddr & 0xFFC0));
	gen(out, vm16_orri(VM16_LW, rd, rd, raddr & 0x003F));
}

static void
asm_addi(struct txt *in, uint16_t  *out)
{
	uint16_t rd = 0, r1 = 0, im7 = 0;

        parse_reg(in, &rd);
        parse_comma(in);
        parse_reg(in, &r1);
        parse_comma(in);
        parse_num(in, 10, &im7);
	gen(out, vm16_orri(VM16_ADDI, rd, r1, im7));
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

void
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
			case TOK_LUI:   asm_lui(in, out);             break;
			case TOK_AUIPC: asm_auipc(in, out);           break;
			case TOK_JALR:  asm_jalr(in, out);            break;
			case TOK_BEQ:   asm_beq(in, out);             break;
			case TOK_LW:    asm_lw(in, out);              break;
			case TOK_SW:    asm_sw(in, out);              break;
			case TOK_ADDI:  asm_addi(in, out); break;
			case TOK_ADD:   asm_math(in, out, VM16_ADD);  break;
			case TOK_SUB:   asm_math(in, out, VM16_SUB);  break;
			case TOK_SLL:   asm_math(in, out, VM16_SLL);  break;
			case TOK_SRL:   asm_math(in, out, VM16_SRL);  break;
			case TOK_NAND:  asm_math(in, out, VM16_NAND); break;
			case TOK_AND:   asm_math(in, out, VM16_AND);  break;
			case TOK_OR:    asm_math(in, out, VM16_OR);   break;
			case TOK_LT:    asm_math(in, out, VM16_LT);   break;
			default:        err(in, &next, "expected instruction");
			}
			get(in);
		}
		txt_reset(in);
	}
}
