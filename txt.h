#ifndef TXT_H__
#define TXT_H__

#include <stdlib.h>
#include <stdio.h>

struct txt {
	char const *name; /* Name of the txt document */
	size_t row;       /* Current row of the cursor */
	size_t col;       /* Current column of the cursor */
	size_t seek;      /* Cursor into the data */
	char const *str;  /* The backing string data */
};

void
txt_init(struct txt *t, char const *name, char const *str);

char
txt_get(struct txt *t);

char const *
txt_at(struct txt *t);

void
txt_reset(struct txt *t);

#endif
