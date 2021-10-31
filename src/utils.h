/**
 * Created: Oct 19, 2020.
 * see: LICENSE.
 */

#pragma once

#define UNUSED(e, ...)      (void) ((void) (e), ##__VA_ARGS__)

#define LIBNAME             "ngram_porter"

// see:
//  https://misc.flogisoft.com/bash/tip_colors_and_formatting
//  https://archive.is/Z9Q2Q
#define _COLOR_RED          "\e[91m"
#define _COLOR_GREEN        "\e[92m"
#define _COLOR_RESET        "\e[0m"

#define _LOG(out, fmt, ...) fprintf(out, "(" LIBNAME ") " fmt "\n", ##__VA_ARGS__)
#define LOG(fmt, ...)       _LOG(stdout, fmt, ##__VA_ARGS__)
#define LOG_DBG(fmt, ...)   LOG("[" _COLOR_GREEN "DBG" _COLOR_RESET "] (%s:%d) " fmt, __BASE_FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)   _LOG(stderr, "[" _COLOR_RED "ERR" _COLOR_RESET "] " fmt, ##__VA_ARGS__)

int parse_int(const char *str, char delim, int base, int *val);

int utf8_char_count(char);
