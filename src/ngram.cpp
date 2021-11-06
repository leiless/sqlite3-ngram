/**
 * SQLite3 FTS5 ngram tokenizer implementation
 *
 * Created: Oct 19, 2020.
 * see: LICENSE.
 */

#include <cstring>
#include <glog/logging.h>
#include <iostream>
#include <algorithm>

#include "sqlite/sqlite3ext.h"      /* Do not use <sqlite3.h>! */

SQLITE_EXTENSION_INIT1

#include "utils.h"
#include "token_vector.h"

/**
 * [qt.]
 * Return a pointer to the fts5_api pointer for database connection db.
 * If an error occurs, return NULL and leave an error in the database
 * handle (accessible using sqlite3_errcode() / errmsg()).
 *
 * see:
 *  https://github.com/thino-rma/fts5_mecab/blob/master/fts5_mecab.c#L32
 *  https://sqlite.org/fts5.html#extending_fts5
 *  https://pspdfkit.com/blog/2018/leveraging-sqlite-full-text-search-on-ios/
 *  https://archive.is/wip/B6gDa
 */
static inline fts5_api *fts5_api_from_db(sqlite3 *db) {
    CHECK_NOTNULL(db);

    fts5_api *pFts5Api = nullptr;
    sqlite3_stmt *pStmt = nullptr;

    if (sqlite3_prepare(db, "SELECT fts5(?1)", -1, &pStmt, nullptr) == SQLITE_OK) {
        if (sqlite3_bind_pointer(pStmt, 1, (void *) &pFts5Api, "fts5_api_ptr", nullptr) == SQLITE_OK) {
            int rc = sqlite3_step(pStmt);
            CHECK_EQ(rc, SQLITE_ROW);
            rc = sqlite3_step(pStmt);
            CHECK_EQ(rc, SQLITE_DONE);
        }
    }

    // [qt.] Invoking sqlite3_finalize() on a NULL pointer is a harmless no-op.
    int rc = sqlite3_finalize(pStmt);
    CHECK_EQ(rc, SQLITE_OK);

    return pFts5Api;
}

// see:
//  7.1. Custom Tokenizers
//  https://sqlite.org/fts5.html#custom_tokenizers

#define MIN_GRAM        1   /* Essentially strstr(3) */
#define MAX_GRAM        4
#define DEFAULT_GRAM    2

typedef struct {
    int ngram;
    bool case_sensitive;
} ngram_tokenizer_t;

/**
 * [qt.]
 *  The final argument is an output variable.
 *  If successful, (*ppOut) should be set to point to the new tokenizer handle and SQLITE_OK returned.
 *  If an error occurs, some value other than SQLITE_OK should be returned.
 *  In this case, fts5 assumes that the final value of *ppOut is undefined.
 */
static int ngram_create(void *pCtx, const char **azArg, int nArg, Fts5Tokenizer **ppOut) {
    DLOG(INFO) << "Creating FTS5 ngram tokenizer ...";
    DLOG(INFO) << "pCtx: " << pCtx << " azArg: " << azArg << " nArg: " << nArg << " ppOut: " << ppOut;

    CHECK_NOTNULL(pCtx);
    CHECK_NOTNULL(azArg);
    CHECK_GE(nArg, 0);
    CHECK_NOTNULL(ppOut);

    auto *pFts5Api = (fts5_api *) pCtx;
    UNUSED(pFts5Api);

    auto *tok = (ngram_tokenizer_t *) sqlite3_malloc(sizeof(ngram_tokenizer_t));
    if (tok == nullptr) {
        LOG(ERROR) << "sqlite3_malloc() fail, size: " << sizeof(*tok);
        return SQLITE_NOMEM;
    }
    (void) memset(tok, 0, sizeof(*tok));

    tok->ngram = DEFAULT_GRAM;
    for (int i = 0; i < nArg; i++) {
        if (!strcmp(azArg[i], "gram")) {
            if (++i >= nArg) {
                LOG(ERROR) << "gram expected one argument, got nothing.";
                goto out_fail;
            }

            int gram;
            if (!parse_int(azArg[i], '\0', 10, &gram)) {
                LOG(ERROR) << "parse_int() fail, str: " << azArg[i];
                goto out_fail;
            }
            if (gram < MIN_GRAM || gram > MAX_GRAM) {
                LOG(ERROR) << gram << "-gram is out of range, should in range [" << MIN_GRAM << ", " << MAX_GRAM << "]";
                goto out_fail;
            }
            tok->ngram = gram;
        } else if (!strcmp(azArg[i], "case_sensitive")) {
            tok->case_sensitive = true;
        } else {
            LOG(ERROR) << "unrecognizable option at index " << i << ": " << azArg[i];
            goto out_fail;
        }
    }

    DLOG(INFO) << "ngram = " << tok->ngram;
    DLOG(INFO) << "case_sensitive = " << tok->case_sensitive;
    *ppOut = (Fts5Tokenizer *) tok;
    return SQLITE_OK;

    out_fail:
    sqlite3_free(tok);
    return SQLITE_ERROR;
}

/**
 * [qt.]
 * This function is invoked to delete a tokenizer handle previously allocated using xCreate().
 * Fts5 guarantees that this function will be invoked exactly once for each successful call to xCreate().
 */
static void ngram_delete(Fts5Tokenizer *pTok) {
    DLOG(INFO) << "Freeing FTS5 " LIBNAME " tokenizer...";

    CHECK_NOTNULL(pTok);
    auto *tok = (ngram_tokenizer_t *) pTok;
    DLOG(INFO) << "pTok: " << tok << " ngram: " << tok->ngram;

    sqlite3_free(tok);
}

typedef int (*xTokenCallback)(
        void *pCtx,         /* Copy of 2nd argument to xTokenize() */
        int tflags,         /* Mask of FTS5_TOKEN_* flags */
        const char *pToken, /* Pointer to buffer containing token */
        int nToken,         /* Size of token in bytes */
        int iStart,         /* Byte offset of token within input text */
        int iEnd            /* Byte offset of end of token within input text */
);

static inline void do_tokenize(
        const std::vector<token> &arr,
        size_t last_index,
        xTokenCallback xToken,
        ngram_tokenizer_t *tok,
        void *pCtx) {
    static size_t first_index = 0;

    int iStart = arr[first_index].get_iStart();
    int iEnd = arr[last_index].get_iEnd();
    CHECK_LT(iStart, iEnd);

    std::stringstream ss;
    for (size_t i = first_index; i <= last_index; i++) {
        ss << arr[i].get_str();
    }
    std::string s = ss.str();

    if (!tok->case_sensitive) {
        // https://stackoverflow.com/questions/313970/how-to-convert-an-instance-of-stdstring-to-lower-case/313990#313990
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    }

    DLOG(INFO) << "> result token = '" << s << "'"
               << " iStart = " << iStart
               << " iEnd = " << iEnd;
    xToken(pCtx, 0, s.c_str(), (int) s.length(), iStart, iEnd);
}

/**
 * [qt.]
 * If an xToken() callback returns any value other than SQLITE_OK,
 *  then the tokenization should be abandoned and the xTokenize() method should immediately return a copy of the xToken() return value.
 * Or, if the input buffer is exhausted, xTokenize() should return SQLITE_OK. Finally,
 *  if an error occurs with the xTokenize() implementation itself,
 *  it may abandon the tokenization and return any error code other than SQLITE_OK or SQLITE_DONE.
 */
static int ngram_tokenize(
        Fts5Tokenizer *pTok,
        void *pCtx,
        int flags,          /* Mask of FTS5_TOKENIZE_* flags */
        const char *pText,
        int nText,
        xTokenCallback xToken) {
    CHECK_NOTNULL(pTok);
    CHECK_NOTNULL(pCtx);
    CHECK_NOTNULL(pText);
    CHECK_GE(nText, 0);
    CHECK_NOTNULL(xToken);

    auto *pFts5Api = (fts5_api *) pCtx;
    UNUSED(pFts5Api);

    auto *tok = (ngram_tokenizer_t *) pTok;
    DLOG(INFO) << tok->ngram << "-gram tokenizing ...";
    DLOG(INFO) << "pTok: " << pTok << " pCtx: " << pCtx << " flags: " << flags;
    // [quote] ... pText may or may not be nul-terminated.
    DLOG(INFO) << "nText: " << nText << " pText: " << std::string(pText, 0, nText);
    DLOG(INFO) << "xToken: " << xToken;

    if (utf8_validatestr(reinterpret_cast<const u_int8_t *>(pText), nText) != 0) {
        LOG(ERROR) << "Met invalid UTF-8 character(s) in the input text, please check the text or issue a bug report";
        return SQLITE_ERROR;
    }

    token_vector tv = token_vector(pText, nText);
    if (!tv.tokenize()) {
        return SQLITE_ERROR;
    }
    for (const token &t: tv.get_tokens()) {
        DLOG(INFO) << "> token = '" << t.get_str()
                   << "' iStart = " << t.get_iStart()
                   << " iEnd = " << t.get_iEnd()
                   << " category = " << t.get_category();
    }

    const std::vector<token> &tokens = tv.get_tokens();

    std::vector<token> prevArr;
    for (size_t i = 0; i < tokens.size(); i++) {
        std::vector<token> arr;

        token_category_t prev_category;
        for (int j = 0; j < tok->ngram; j++) {
            // Avoid out of array boundary
            if (i + j >= tokens.size()) {
                if (tokens.size() >= (size_t) tok->ngram) {
                    bool same_category = true;

                    for (int k = 0; k < tok->ngram; k++) {
                        token_category_t category = tokens[tokens.size() - k - 1].get_category();
                        if (k != 0) {
                            if (category != prev_category) {
                                same_category = false;
                                break;
                            }
                        }
                        prev_category = category;
                    }

                    // Same category meaning previously last ngram token had been added
                    // Thus we don't need to cut again(unless they're in different categories)
                    if (same_category) {
                        DLOG(INFO)
                                << "Don't do tokenize for the last N non-complete terms since they're in a same category";
                        arr.clear();
                    }
                }

                break;
            }

            const token &curr_token = tokens[i + j];

            if (j != 0) {
                if (curr_token.get_category() != OTHER) {
                    break;
                }
                if (curr_token.get_category() != prev_category) {
                    break;
                }
            }

            arr.emplace_back(curr_token);
            prev_category = curr_token.get_category();
        }

        if (!arr.empty()) {
            // Temporarily solution to the input text case 'Hello世界'
            if (prevArr.size() == 1 && prevArr[0].get_category() != OTHER && arr[0].get_category() == OTHER) {
                for (size_t u = 0; u + 1 < arr.size(); u++) {
                    DLOG(INFO) << "--- " << (u + 1);
                    for (size_t v = 0; v <= u; v++) {
                        do_tokenize(arr, v, xToken, tok, pCtx);
                    }
                }
            }

            do_tokenize(arr, arr.size() - 1, xToken, tok, pCtx);

            prevArr = std::move(arr);
        }
    }

    return SQLITE_OK;
}

static fts5_tokenizer token_handle = {
        .xCreate = ngram_create,
        .xDelete = ngram_delete,
        .xTokenize = ngram_tokenize,
};

/**
 * SQLite loadable extension entry point
 * see:
 *  https://www.sqlite.org/loadext.html#programming_loadable_extensions
 *  https://sqlite.org/fts5.html#extending_fts5
 */
#ifdef _WIN32
__declspec(dllexport)
#else
extern "C"
#endif
UNUSED_ATTR
int sqlite3_ngram_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi) {
    google::InstallFailureSignalHandler();

    CHECK_NOTNULL(db);
    CHECK_NOTNULL(pzErrMsg);
    CHECK_NOTNULL(pApi);

    // Initialize the global sqlite3_api variable.
    //  so all sqlite3_*() functions can be used.
    SQLITE_EXTENSION_INIT2(pApi)

    LOG(INFO) << "HEAD commit: " << BUILD_HEAD_COMMIT;
    LOG(INFO) << "Built by " << BUILD_USER << " at " << BUILD_TIMESTAMP;
    LOG(INFO) << "SQLite3 compile-time version: " << SQLITE_VERSION;
    LOG(INFO) << "SQLite3 run-time version: " << sqlite3_libversion();

    fts5_api *pFts5Api = fts5_api_from_db(db);
    if (pFts5Api == nullptr) {
        int err = sqlite3_errcode(db);
        CHECK_NE(err, 0);
        *pzErrMsg = sqlite3_mprintf("%s(): err: %d msg: %s", __func__, err, sqlite3_errstr(err));
        return err;
    }
    CHECK_EQ(pFts5Api->iVersion, 2);

    return pFts5Api->xCreateTokenizer(pFts5Api, LIBNAME, (void *) pFts5Api, &token_handle, nullptr);
}
