#include "token_vector.h"

#include <cctype>
#include <glog/logging.h>
#include <iostream>
#include <utility>

namespace ngram_tokenizer {
    Token::Token(std::string str, int iStart, int iEnd, token_category_t category) {
        CHECK_GE(iStart, 0);
        CHECK_GE(iEnd, 0);
        CHECK_LT(iStart, iEnd);

        this->str = std::move(str);
        this->iStart = iStart;
        this->iEnd = iEnd;
        this->category = category;
    }

    const std::string &Token::get_str() const {
        return str;
    }

    int Token::get_iStart() const {
        return iStart;
    }

    int Token::get_iEnd() const {
        return iEnd;
    }

    token_category_t Token::get_category() const {
        return category;
    }

    TokenVector::TokenVector(const char *pText, int nText) {
        CHECK_NOTNULL(pText);
        CHECK_GE(nText, 0);
        this->pText = pText;
        this->nText = nText;
        this->ok = false;
    }

    bool TokenVector::tokenize() {
        int iStart = 0;
        int iEnd = 0;

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
                // Will properly null-terminate the resulting std::string
                std::string s(pText + iStart, iEnd - iStart);
                tokens.emplace_back(s, iStart, iEnd, category);
            }

            iStart = iEnd;
        }

        if (iEnd != nText) {
            // Certainly not a valid UTF-8 string
            return false;
        }
        ok = true;
        return true;
    }

// Call only after a successful call of tokenize()
    const std::vector<Token> &TokenVector::get_tokens() const {
        CHECK(ok) << "Make sure tokenize() has been successfully called";
        return tokens;
    }

    token_category_t TokenVector::token_category(char c) {
        if (isdigit(c)) {
            return DIGIT;
        }
        if (isspace(c) || iscntrl(c)) {
            return SPACE_OR_CONTROL;
        }
        if (isalpha(c)) {
            return ALPHABETIC;
        }
        if (ispunct(c)) {
            return PUNCTUATION;
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
 *  https://github.com/apple/darwin-xnu/blob/main/bsd/vfs/vfs_utfconv.c#L662
 */
    int TokenVector::utf8_char_count(char c) {
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
}
