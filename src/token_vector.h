#pragma once

#include <string>
#include <vector>

typedef enum {
    DIGIT,
    SPACE_OR_CONTROL,
    ALPHABETIC,
    OTHER
} token_category_t;

class token {
public:
    token(std::string, int, int, token_category_t);

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

class token_vector {
public:
    token_vector(const char *, int);

    bool tokenize();

    const std::vector<token> &get_tokens() const;

private:
    static token_category_t token_category(char);

    static int utf8_char_count(char);

    const char *pText;
    int nText;
    std::vector<token> tokens;
};
