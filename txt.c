/* See LICENSE txt for copyright and license details */
#include "txt.h"

void
txt_init(struct txt *t, char const *name, char const *str)
{
	t->name = name;
	t->row = 1;
	t->col = 1;
	t->seek = 0;
	t->str = str;
}

char const *
txt_at(struct txt *t)
{
	return &t->str[t->seek];
}

char
txt_get(struct txt *t)
{
	int ch;

	if ('\0' == t->str[t->seek]) {
		return '\0';
	}
	ch = t->str[t->seek++];
	if ('\n' == ch) {
		t->row += 1;
		t->col = 1;
	} else {
		t->col += 1;
	}
	return ch;
}

void
txt_reset(struct txt *t)
{
	txt_init(t, t->name, t->str);
}
