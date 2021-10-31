#pragma once

#include <string>
#include <vector>

class token {
public:
    token(const char *, int, int);

    const std::string &get_str() const;

    int get_iStart() const;

    int get_iEnd() const;

private:
    std::string str;
    int iStart;
    int iEnd;
};

class token_vector {
public:
    token_vector(const char *, int);

    bool tokenize();

    const std::vector<token> &get_tokens() const;

private:
    typedef enum {
        DIGIT,
        SPACE_OR_CONTROL,
        ALPHABETIC,
        OTHER
    } token_category_t;

    static token_category_t token_category(char);

    static int utf8_char_count(char);

    const char *pText;
    int nText;
    std::vector<token> tokens;
};
