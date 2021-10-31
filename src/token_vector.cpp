#include "token_vector.h"

#include <cctype>
#include <glog/logging.h>
#include <iostream>
#include <utility>

token::token(std::string str, int iStart, int iEnd) {
    CHECK_GE(iStart, 0);
    CHECK_GE(iEnd, 0);
    CHECK_LT(iStart, iEnd);

    this->str = std::move(str);
    this->iStart = iStart;
    this->iEnd = iEnd;
}

const std::string &token::get_str() const {
    return str;
}

int token::get_iStart() const {
    return iStart;
}

int token::get_iEnd() const {
    return iEnd;
}

token_vector::token_vector(const char *pText, int nText) {
    CHECK_NOTNULL(pText);
    CHECK_GE(nText, 0);
    this->pText = pText;
    this->nText = nText;
}


bool token_vector::tokenize() {
    int iStart = 0;
    int iEnd = 0;
    int nthToken = 0;

    while (iEnd < nText) {
        token_category_t category = token_category(pText[iEnd]);
        if (category == OTHER) {
            int len = utf8_char_count(pText[iEnd]);
            if (len <= 0) {
                LOG(ERROR) << "Met non-UTF8 character at index " << iEnd;
                return false;
            }
            iEnd += len;
        } else {
            while (++iEnd < nText && token_category(pText[iEnd]) == category) {
                // continue
            }
        }

        if (category != SPACE_OR_CONTROL) {
            nthToken++;

            // Will properly null-terminate the resulting std::string
            std::string s(pText + iStart, iEnd - iStart);
            tokens.emplace_back(s, iStart, iEnd);
        }

        iStart = iEnd;
    }

    if (iEnd != nText) {
        // Certainly not a valid UTF-8 string
        return false;
    }
    return true;
}

// Call only after a successful call of tokenize()
const std::vector<token> &token_vector::get_tokens() const {
    return tokens;
}

token_vector::token_category_t token_vector::token_category(char c) {
    if (isdigit(c)) {
        return DIGIT;
    }
    if (isspace(c) || iscntrl(c)) {
        return SPACE_OR_CONTROL;
    }
    if (isalpha(c)) {
        return ALPHABETIC;
    }
    return OTHER;
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
int token_vector::utf8_char_count(char c) {
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
