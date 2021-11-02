/**
 * Created: Oct 19, 2020.
 * see: LICENSE.
 */

#pragma once

#include <sys/types.h>
#include <cstddef>

#define UNUSED(e, ...)      (void) ((void) (e), ##__VA_ARGS__)
#define UNUSED_ATTR         __attribute__((unused))
#define LIBNAME             "ngram"

int parse_int(const char *, char, int, int *);

int utf8_validatestr(const u_int8_t *, size_t);
