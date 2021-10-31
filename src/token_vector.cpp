#include "token_vector.h"

#include <glog/logging.h>

token::token(const char *str, int iStart, int iEnd) {
    this->str = str;
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

}

const std::vector<token> &token_vector::get_tokens() const {
    return tokens;
}
