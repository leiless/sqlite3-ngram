#include <cerrno>
#include <cstdlib>
#include <cstdint>

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

int parse_int(const char *str, char delim, int base, int *val) {
    long n;
    int ok = parse_long(str, delim, base, &n);
    if (ok) {
        if (((uint64_t) n) & ~0xffffffffllu) {
            errno = ERANGE;
            ok = 0;
        } else {
            *val = (int) n;
        }
    }
    return ok;
}

/**
 * Count how many bytes an UTF-8 character occupied
 *
 * @c       The UTF8 character starting code point
 * @return  0 if it's not a valid UTF-8 character
 *
 * see:
 *  https://en.wikipedia.org/wiki/UTF-8#Encoding
 *  https://stackoverflow.com/questions/64846096/utf-8-character-count/64846299#64846299
 *  https://xr.anadoxin.org/source/xref/macos-10.14.1-mojave/xnu-4903.221.2/bsd/vfs/vfs_utfconv.c#639
 *
 * TODO: introduce an UTF-8 library for robust determination
 */
int utf8_char_count(char c) {
    int n = 0;
    while ((c & 0x80) && n < 4) {
        n++;
        c <<= 1;
    }
    if (n == 1) {
        return 0;
    }
    return n ? n : 1;
}
