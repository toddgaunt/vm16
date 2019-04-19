/* See LICENSE file for copyright and license details */
#ifndef VM16_H
#define VM16_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Opcodes */
#define VM16_LUI    0x0
#define VM16_AUIPC  0x1
#define VM16_JALR   0x2
#define VM16_BEQ    0x3
#define VM16_LW     0x4
#define VM16_SW     0x5
#define VM16_ADDI   0x6
#define VM16_MATH   0x7

/* Altcodes for MATH instructions */
#define VM16_ADD    0x0
#define VM16_SUB    0x1
#define VM16_SLL    0x2
#define VM16_SRL    0x3
#define VM16_NAND   0x4
#define VM16_AND    0x5
#define VM16_OR     0x6
#define VM16_LT     0x7

/* Significant memory addresses */
#define VM16_ADDR_HALT  0x0000
#define VM16_ADDR_OUT   0x0001
#define VM16_ADDR_IN    0x0002
#define VM16_ADDR_START 0x0010

/* Maximum amount of memory available */
#define VM16_MM_SIZE (1 << 15)

struct vm16 {
	uint16_t ir;               /* Instruction register */
	uint16_t pc : 15;          /* Program counter */
	uint16_t r[8];             /* General purpose registers */
	uint16_t mm[VM16_MM_SIZE]; /* Main memory */
};


/* Dump a text representation of machine state to file */
void
vm16_dump(FILE *out, struct vm16 const *v);

/* Execute until the program counter equals 0 */
void
vm16_exec(struct vm16 *vm);

void
vm16_init(struct vm16 *v);

bool
vm16_load(struct vm16 *vm, uint16_t *program, uint16_t n);

/* Synthesize an ori type instruction */
uint16_t
vm16_ori(uint8_t op, uint8_t rd, uint16_t im10);

/* Synthesize an orri type instruction */
uint16_t
vm16_orri(uint8_t op, uint8_t rd, uint8_t r1, uint8_t im7);

/* Synthesize an orrar type instruction */
uint16_t
vm16_orrar(uint8_t op, uint8_t rd, uint8_t r1, uint8_t alt, uint8_t r2);

/* Execute a single fetch->decode->execute cycle */
void
vm16_step(struct vm16 *vm);

#endif
