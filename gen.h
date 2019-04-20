#ifndef GEN_H__
#define GEN_H__

#include "txt.h"
#include "vm16.h"

size_t
assemble(struct txt *in, uint16_t out[VM16_MM_SIZE]);

#endif
