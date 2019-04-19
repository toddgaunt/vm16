/* See LICENSE file for copyright and license details */
#ifndef LOG_H__
#define LOG_H__

#include <stdarg.h>
#include <stdio.h>

enum {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
	LOG_OFF,
	LOG_COUNT,
};

#define log_trace(...)  _log(LOG_TRACE, stderr, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...)  _log(LOG_DEBUG, stderr, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)   _log(LOG_INFO,  stdout, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)   _log(LOG_WARN,  stderr, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...)  _log(LOG_ERROR, stderr, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...)  _log(LOG_FATAL, stderr, __FILE__, __LINE__, __VA_ARGS__)

int
log_set_level(int level);

void
_log(int level, FILE *out, char const *file, int row, char const *fmt, ...);

#endif
