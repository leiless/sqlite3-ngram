#include <cerrno>
#include <cstdlib>
#include <cstdint>
#include <sstream>

#include "utils.h"

namespace ngram_tokenizer {
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

    // Taken from https://github.com/apple/darwin-xnu/blob/main/bsd/vfs/vfs_utfconv.c#L662
    //  with modification

    static int utf_extrabytes[32] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 2, 2, 3, -1
    };

/* Surrogate Pair Constants */
#define SP_HALF_SHIFT   10
#define SP_HALF_BASE    0x0010000u
#define SP_HALF_MASK    0x3FFu

#define SP_HIGH_FIRST   0xD800u
#define SP_HIGH_LAST    0xDBFFu
#define SP_LOW_FIRST    0xDC00u
#define SP_LOW_LAST             0xDFFFu

    /*
     * utf8_validatestr - Check for a valid UTF-8 string.
     * @return      0 if valid, EINVAL if invalid
     */
    int
    utf8_validatestr(const u_int8_t *utf8p, size_t utf8len) {
        unsigned int byte;
        u_int32_t ch;
        unsigned int ucs_ch;
        size_t extrabytes;

        while (utf8len-- > 0 && (byte = *utf8p++) != '\0') {
            if (byte < 0x80) {
                continue;  /* plain ascii */
            }
            extrabytes = (unsigned char) utf_extrabytes[byte >> 3];

            if (utf8len < extrabytes) {
                goto invalid;
            }
            utf8len -= extrabytes;

            switch (extrabytes) {
                case 1:
                    ch = byte;
                    ch <<= 6;   /* 1st byte */
                    byte = *utf8p++;       /* 2nd byte */
                    if ((byte >> 6) != 2) {
                        goto invalid;
                    }
                    ch += byte;
                    ch -= 0x00003080UL;
                    if (ch < 0x0080) {
                        goto invalid;
                    }
                    break;
                case 2:
                    ch = byte;
                    ch <<= 6;   /* 1st byte */
                    byte = *utf8p++;       /* 2nd byte */
                    if ((byte >> 6) != 2) {
                        goto invalid;
                    }
                    ch += byte;
                    ch <<= 6;
                    byte = *utf8p++;       /* 3rd byte */
                    if ((byte >> 6) != 2) {
                        goto invalid;
                    }
                    ch += byte;
                    ch -= 0x000E2080UL;
                    if (ch < 0x0800) {
                        goto invalid;
                    }
                    if (ch >= 0xD800) {
                        if (ch <= 0xDFFF) {
                            goto invalid;
                        }
                        if (ch == 0xFFFE || ch == 0xFFFF) {
                            goto invalid;
                        }
                    }
                    break;
                case 3:
                    ch = byte;
                    ch <<= 6;   /* 1st byte */
                    byte = *utf8p++;       /* 2nd byte */
                    if ((byte >> 6) != 2) {
                        goto invalid;
                    }
                    ch += byte;
                    ch <<= 6;
                    byte = *utf8p++;       /* 3rd byte */
                    if ((byte >> 6) != 2) {
                        goto invalid;
                    }
                    ch += byte;
                    ch <<= 6;
                    byte = *utf8p++;       /* 4th byte */
                    if ((byte >> 6) != 2) {
                        goto invalid;
                    }
                    ch += byte;
                    ch -= 0x03C82080UL + SP_HALF_BASE;
                    ucs_ch = (ch >> SP_HALF_SHIFT) + SP_HIGH_FIRST;
                    if (ucs_ch < SP_HIGH_FIRST || ucs_ch > SP_HIGH_LAST) {
                        goto invalid;
                    }
                    ucs_ch = (ch & SP_HALF_MASK) + SP_LOW_FIRST;
                    if (ucs_ch < SP_LOW_FIRST || ucs_ch > SP_LOW_LAST) {
                        goto invalid;
                    }
                    break;
                default:
                    goto invalid;
            }
        }
        return 0;
        invalid:
        return EINVAL;
    }

    // https://stackoverflow.com/questions/9435385/split-a-string-using-c11
    std::vector<std::string> split(const std::string &s, char delim) {
        std::stringstream ss(s);
        std::string item;
        std::vector<std::string> elems;
        while (std::getline(ss, item, delim)) {
            elems.emplace_back(item);
        }
        return elems;
    }

    static const char *WHITESPACE = " \n\r\t\f\v";

    static inline std::string ltrim(const std::string &s) {
        size_t start = s.find_first_not_of(WHITESPACE);
        return start == std::string::npos ? "" : s.substr(start);
    }

    static inline std::string rtrim(const std::string &s) {
        size_t end = s.find_last_not_of(WHITESPACE);
        return end == std::string::npos ? "" : s.substr(0, end + 1);
    }

    // https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/
    std::string trim(const std::string &s) {
        return rtrim(ltrim(s));
    }
}
