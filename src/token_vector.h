#pragma once

#include <string>
#include <vector>

namespace ngram_tokenizer {
    typedef enum {
        DIGIT,
        SPACE_OR_CONTROL,
        ALPHABETIC,
        PUNCTUATION,
        OTHER
    } token_category_t;

    class Token {
    public:
        Token(std::string, int, int, token_category_t);

        const std::string &get_str() const;

        int get_iStart() const;

        int get_iEnd() const;

        token_category_t get_category() const;

    private:
        std::string str;
        int iStart; // Inclusive
        int iEnd; // Exclusive
        token_category_t category;
    };

    class TokenVector {
    public:
        TokenVector(const char *, int);

        bool tokenize();

        const std::vector<Token> &get_tokens() const;

    private:
        static token_category_t token_category(char);

        static int utf8_char_count(char);

        const char *pText;
        int nText;
        std::vector<Token> tokens;
        bool ok;
    };
}
