/* See LICENSE file for copyright and license details */
#ifndef ZONE_H__
#define ZONE_H__

#include <stdlib.h>

typedef struct zone zone;

/*
 * zone_push pushes a new region onto the global region stack
 */
void
zone_push();

void
zone_pop();

void *
zone_alloc(size_t n);

zone *
zone_pushz(zone *z);

zone *
zone_popz(zone *z);

void *
zone_allocz(zone *z, size_t n);

#endif
