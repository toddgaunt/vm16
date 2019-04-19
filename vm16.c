/* See LICENSE file for copyright and license details */
#include <string.h>

#include "vm16.h"

#define M3(x) ((x) & 0x7)
#define M7(x) ((x) & 0x7F)
#define MA(x) ((x) & 0x3FF)

void
vm16_dump(FILE *out, struct vm16 const *v)
{
	fprintf(out, "ir: 0x%x\n", v->ir);
	fprintf(out, "pc: 0x%x\n", v->pc);
	fprintf(out, "r:  [0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]\n",
			v->r[0], v->r[1], v->r[2], v->r[3],
			v->r[4], v->r[5], v->r[6], v->r[7]);
}

void
vm16_exec(struct vm16 *v)
{
	while (v->pc != VM16_ADDR_HALT)
		vm16_step(v);
}

void
vm16_init(struct vm16 *v)
{
	memset(v, 0, sizeof(*v));
	v->pc = 0x10;
}

bool
vm16_loadf(struct vm16 *vm, FILE *in)
{
	int ch;
	size_t i;
	uint16_t instr;

	for (i = 0; i < VM16_MM_SIZE; ++i) {
		ch = getc(in);
		if (ch == EOF)
			break;
		instr = ch << 8;
		ch = getc(in);
		if (ch == EOF) {
			printf("UNEXPECTED EOF\n");
			exit(-1);
		}
		instr |= ch;
		vm->mm[0x10 + i] = instr;
	}
	return true;
}

bool
vm16_load(struct vm16 *vm, uint16_t *words, uint16_t nwords)
{
	/* Don't load the words if they can't fit in main memory */
	if (0x10 + nwords >= VM16_MM_SIZE)
		return false;
	memcpy(&vm->mm[0x10], words, nwords);
	return true;
}

uint16_t
vm16_ori(uint8_t op, uint8_t rd, uint16_t im10)
{
	return MA(im10) << 6 | M3(rd) << 3 | M3(op);
}

uint16_t
vm16_orri(uint8_t op, uint8_t rd, uint8_t r1, uint8_t im7)
{
	return M7(im7) << 9 | M3(r1) << 6 | M3(rd) << 3 | M3(op);
}

uint16_t
vm16_orrar(uint8_t op, uint8_t rd, uint8_t r1, uint8_t alt, uint8_t r2)
{
	return M3(r2) << 13 | M3(alt) << 9 | M3(r1) << 6 | M3(rd) << 3 | M3(op);
}

void
vm16_step(struct vm16 *v)
{
	uint16_t op, rd, im10, r1, im7, alt, r2;
	
	if (v->pc == VM16_ADDR_HALT)
		return;
	/* Fetch */
	v->ir = v->mm[v->pc++];
	/* Decode */
	op   = (v->ir & 0x0007) >> 0;
	rd   = (v->ir & 0x0038) >> 3;
	im10 = (v->ir & 0xFFC0) >> 6;
	r1   = (v->ir & 0x01C0) >> 6;
	im7  = (v->ir & 0xFE00) >> 9;
	alt  = (v->ir & 0x1E00) >> 9;
	r2   = (v->ir & 0xE000) >> 13;
	/* Sign extend the 7-bit immediate */
	im7 |= im7 & 0x40 ? 0xFF80 : 0x0000;
	/* Execute */
	switch(op) {
	case VM16_LUI:   v->r[rd] = im10 << 6;
	break;
	case VM16_AUIPC: v->r[rd] = v->pc + (im10 << 6);
	break;
	case VM16_JALR:  if (rd) v->r[rd] = v->pc + 1; v->pc = v->r[r1] + im7;
	break;
	case VM16_BEQ:   v->pc += v->r[rd] == v->r[r1] ? im7 : 0;
	break;
	case VM16_LW:    v->r[rd] = v->mm[v->r[r1] + im7];
	break;
	case VM16_SW:    v->mm[v->r[r1] + im7] = v->r[rd];
	break;
	case VM16_ADDI:  v->r[rd] = v->r[r1] + im7;
	break;
	case VM16_MATH:
		switch (alt) {
		case VM16_ADD:  v->r[rd] = v->r[r1] + v->r[r2];
		break;
		case VM16_SUB:  v->r[rd] = v->r[r1] - v->r[r2];
		break;
		case VM16_SLL:  v->r[rd] = v->r[r1] << v->r[r2];
		break;
		case VM16_SRL:  v->r[rd] = v->r[r1] >> v->r[r2];
		break;
		case VM16_NAND: v->r[rd] = ~(v->r[r1] & v->r[r2]);
		break;
		case VM16_AND:  v->r[rd] = v->r[r1] & v->r[r2];
		break;
		case VM16_OR:   v->r[rd] = v->r[r1] | v->r[r2];
		break;
		case VM16_LT:   v->r[rd] = v->r[r1] < v->r[r2];
		break;
		}
	}
	/* Hardwire register zero to the value 0 */
	v->r[0] = 0;
	/* Output anything written to memory-mapped stdout */
	if (v->mm[VM16_ADDR_OUT]) {
		putc(v->mm[VM16_ADDR_OUT], stdout);
		v->mm[VM16_ADDR_OUT] = 0;
	}
}
