/* See LICENSE file for copyright and license details */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "zone.h"

#define CHUNK_SIZE 4096

struct zone *head;

struct zone {
	struct zone *next;
	struct chunk *chunk;
};

struct chunk {
	struct chunk *next;
	size_t sp;
	uint8_t *bytes;
};

static size_t
max(size_t a, size_t b) {return a > b ? a : b;}

static struct chunk *
mkchunk(size_t size, struct chunk *next)
{
	struct chunk *rv;

	rv = calloc(1, sizeof(*rv) + size);
	if (!rv)
		return NULL;
	rv->bytes = (uint8_t *)(rv + 1);
	rv->sp = size;
	rv->next = next;

	return rv;
}

void
zone_push()
{
	head = zone_pushz(head);
}

void
zone_pop()
{
	head = zone_popz(head);
}

void *
zone_alloc(size_t n)
{
	if (!head)
		zone_push();
	return zone_allocz(head, n);
}

zone *
zone_pushz(zone *z)
{
	struct zone *tmp;

	tmp = malloc(sizeof(*tmp));
	tmp->next = z;

	/* The initial zone is empty to avoid allocating unused space */
	tmp->chunk = mkchunk(0, NULL);
	return tmp;
}

zone *
zone_popz(zone *z)
{
	struct chunk *cp;
	struct zone *next;

	if (!z) {
		fprintf(stderr, "Cannot pop, no more zone\n");
		exit(-1);
	}

	/* Release all chunks before losing access to them. */

	cp = z->chunk;
	while (cp) {
		struct chunk *garbage;
			
		garbage = cp;
		cp = cp->next;
		free(garbage);
	}

	next = z->next;
	free(z);
	return next;
}

void *
zone_allocz(zone *z, size_t n)
{
	/* Align to 32 bytes */
	n = (n + 31) & ~31;

	if (n > z->chunk->sp) {
		struct chunk *tmp;

		tmp = mkchunk(max(n, CHUNK_SIZE), z->chunk);
		if (!tmp)
			return NULL;
		z->chunk = tmp;
	}

	z->chunk->sp -= n;
	return &z->chunk->bytes[z->chunk->sp];
}
