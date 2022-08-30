/* See LICENSE file for copyright and license details */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zone.h"
#include "symtab.h"

static size_t
hash(void const *k, size_t klen)
{
	size_t i;
	size_t const p = 16777619;
	size_t hash = 2166136261u;

	for (i = 0; i < klen; ++i)
		hash = (hash ^ ((uint8_t *)k)[i]) * p;
	hash += hash << 13;
	hash ^= hash >> 7;
	hash += hash << 3;
	hash ^= hash >> 17;
	hash += hash << 5;
	return hash;
}

static size_t
find_index(struct symtab const *st, struct token const *k)
{
	size_t i;
	size_t begin;
	
	begin = hash(k->bytes, k->len) % st->size;
	i = begin;
	while (st->k[i]) {
		if (st->k[i]->len == k->len && !memcmp(st->k[i]->bytes, k->bytes, k->len)) {
			break;
		}
		i = (i + 1) % st->size;
		if (begin == i) {
			return st->size;
		}
	}
	return i;
}

struct symtab *
symtab_create(size_t size)
{
	struct zone *z;
	struct symtab *st;
		
	z = zone_pushz(NULL);

	st = zone_allocz(z, sizeof(*st));
	if (!st) {
		return NULL;
	}
	st->z = z;
	st->k = zone_allocz(z, sizeof(*st->k) * size);
	st->v = zone_allocz(z, sizeof(*st->v) * size);
	st->size = size;
	st->used = 0;
	return st;
}

void
symtab_destroy(struct symtab *s)
{
	zone_popz(s->z);
}

struct token const *
symtab_getk(struct symtab const *st, struct token const *k)
{
	size_t i = find_index(st, k);

	if (i >= st->size || !st->k[i]) {
		return NULL;
	}
	return st->k[i];
}

uint16_t *
symtab_getv(struct symtab *st, struct token const *k)
{
	size_t i = find_index(st, k);

	if (i >= st->size) {
		return NULL;
	}
	if (!st->k[i]) {
		st->k[i] = zone_allocz(st->z, sizeof(*k));
		if (!st->k[i])
			return NULL;
		memcpy(st->k[i], k, sizeof(*k));
		st->used += 1;
	}
	return &st->v[i];
}

uint16_t *
symtab_at(struct symtab const *st, struct token const *k)
{
	size_t i = find_index(st, k);

	if (i >= st->size || !st->k[i]) {
		return NULL;
	}
	return &st->v[i];
}
