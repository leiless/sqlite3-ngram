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

    std::pair<bool, std::string> tokenize();

    const std::vector<token> &get_tokens() const;

private:
    std::vector<token> tokens;
};
