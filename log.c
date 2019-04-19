/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "log.h"

static char *msg[] = {"trace", "debug", "info", "warn", "error", "fatal"};

static int log_level = LOG_INFO;

int
log_set_level(int level)
{
	int old = log_level;
	log_level = level;
	return old;
}

void
_log(int level, FILE *out, char const *file, int row, char const *fmt, ...)
{
	va_list ap;

	if (log_level != LOG_OFF) {
		/* Logging at debug levels gives more detail for developers */
		if (log_level < LOG_INFO)
			fprintf(out, "%-5s %s:%d: ", msg[level], file, row);
		va_start(ap, fmt);
		vfprintf(out, fmt, ap);
		va_end(ap);
		fflush(out);
	}
	/* Fatal messages indicate the program cannot continue to run */
	if (LOG_FATAL == level)
		exit(-1);
}
