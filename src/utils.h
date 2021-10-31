/**
 * Created: Oct 19, 2020.
 * see: LICENSE.
 */

#pragma once

#define UNUSED(e, ...)      (void) ((void) (e), ##__VA_ARGS__)
#define UNUSED_ATTR         __attribute__((unused))
#define LIBNAME             "ngram_porter"

int parse_int(const char *str, char delim, int base, int *val);
