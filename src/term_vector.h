#pragma once

#include <string>
#include <vector>

class term {
public:
    term(const char *, int, int);

    const std::string &get_str() const;

    int get_iStart() const;

    int get_iEnd() const;

private:
    std::string str;
    int iStart;
    int iEnd;
};

class term_vector {
public:
    term_vector(const char *, int);

    const std::vector<term> &get_terms() const;

private:
    std::vector<term> terms;
};
