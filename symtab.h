/* See LICENSE file for copyright and license details */
#ifndef SYMTAB_H__
#define SYMTAB_H__

#include "lex.h"

struct symtab {
	struct zone *z;
	struct token **k;
	uint16_t *v;
	size_t size;
	size_t used;
};

struct symtab *
symtab_create(size_t size);

struct token const *
symtab_getk(struct symtab const *st, struct token const *k);

uint16_t *
symtab_getv(struct symtab *st, struct token const *k);

uint16_t *
symtab_at(struct symtab const *st, struct token const *k);

#endif
