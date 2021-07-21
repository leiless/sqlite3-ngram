#include <errno.h>
#include <stdlib.h>

#include "utils.h"

/**
 * [sic strtol(3)] Convert a string value to a long
 *
 * @param str       the value string
 * @param delim     delimiter character(an invalid one) for a success match
 *                  note '\0' for a strict match
 *                  other value indicate a substring conversion
 * @param base      numeric base
 * @param val       where to store parsed long value
 * @return          1 if parsed successfully, 0 otherwise.
 *
 * @see:            https://stackoverflow.com/a/14176593/10725426
 */
static int parse_long(const char *str, char delim, int base, long *val) {
    char *p;
    errno = 0;
    long n = strtol(str, &p, base);
    int ok = errno == 0 && *p == delim;
    if (ok) *val = n;
    return ok;
}

int parse_u32(const char *str, char delim, int base, u32 *val) {
    long n;
    int ok = parse_long(str, delim, base, &n);
    if (ok) {
        if ((uint64_t) n & ~0xffffffffllu) {
            errno = ERANGE;
            ok = 0;
        } else {
            *val = (uint32_t) n;
        }
    }
    return ok;
}
