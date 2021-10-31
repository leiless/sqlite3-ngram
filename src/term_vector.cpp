#include "term_vector.h"

#include <glog/logging.h>

term::term(const char *str, int iStart, int iEnd) {
    this->str = str;
    this->iStart = iStart;
    this->iEnd = iEnd;
}

const std::string &term::get_str() const {
    return str;
}

int term::get_iStart() const {
    return iStart;
}

int term::get_iEnd() const {
    return iEnd;
}

term_vector::term_vector(const char *pText, int nText) {
    CHECK_NOTNULL(pText);
    CHECK_GE(nText, 0);

}

const std::vector<term> &term_vector::get_terms() const {
    return terms;
}
