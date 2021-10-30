/**
 * SQLite3 FTS5 ngram tokenizer implementation
 *
 * Created: Oct 19, 2020.
 * see: LICENSE.
 */

#include <string.h>
#include <ctype.h>

#include "sqlite/sqlite3ext.h"      /* Do not use <sqlite3.h>! */

SQLITE_EXTENSION_INIT1

#define ASSERTF_DEF_ONCE

#include "assertf.h"
#include "utils.h"
#include "types.h"

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
static fts5_api *fts5_api_from_db(sqlite3 *db) {
    assert_nonnull(db);

    fts5_api *pFts5Api = NULL;
    sqlite3_stmt *pStmt = NULL;

    if (sqlite3_prepare(db, "SELECT fts5(?1)", -1, &pStmt, 0) == SQLITE_OK) {
        if (sqlite3_bind_pointer(pStmt, 1, (void *) &pFts5Api, "fts5_api_ptr", NULL) == SQLITE_OK) {
            int rc = sqlite3_step(pStmt);
            assert_eq(rc, SQLITE_ROW, %d);
            rc = sqlite3_step(pStmt);
            assert_eq(rc, SQLITE_DONE, %d);
        }
    }

    // [qt.] Invoking sqlite3_finalize() on a NULL pointer is a harmless no-op.
    int rc = sqlite3_finalize(pStmt);
    assert_eq(rc, SQLITE_OK, %d);

    return pFts5Api;
}

// see:
//  7.1. Custom Tokenizers
//  https://sqlite.org/fts5.html#custom_tokenizers

#define MIN_GRAM        1   /* Essentially strstr(3) */
#define MAX_GRAM        4
#define DEFAULT_GRAM    2

typedef struct {
    u32 ngram;
} ngram_tokenizer_t;

/**
 * [qt.]
 *  The final argument is an output variable.
 *  If successful, (*ppOut) should be set to point to the new tokenizer handle and SQLITE_OK returned.
 *  If an error occurs, some value other than SQLITE_OK should be returned.
 *  In this case, fts5 assumes that the final value of *ppOut is undefined.
 */
static int ngram_create(void *pCtx, const char **azArg, int nArg, Fts5Tokenizer **ppOut) {
    LOG_DBG("Creating FTS5 ngram tokenizer...");
    LOG_DBG("pCtx: %p azArg: %p nArg: %d ppOut: %p", pCtx, azArg, nArg, ppOut);

    assert_nonnull(pCtx);
    assert_nonnull(azArg);
    assert_ge(nArg, 0, %d);
    assert_nonnull(ppOut);

    fts5_api *pFts5Api = (fts5_api *) pCtx;
    UNUSED(pFts5Api);

    ngram_tokenizer_t *tok = sqlite3_malloc(sizeof(*tok));
    if (tok == NULL) {
        LOG_ERR("sqlite3_malloc() fail, size: %zu", sizeof(*tok));
        return SQLITE_NOMEM;
    }
    (void) memset(tok, 0, sizeof(*tok));

    tok->ngram = DEFAULT_GRAM;
    for (int i = 0; i < nArg; i++) {
        if (!strcmp(azArg[i], "gram")) {
            if (++i >= nArg) {
                LOG_ERR("gram expected one argument, got nothing.");
                goto out_fail;
            }

            u32 gram;
            if (!parse_u32(azArg[i], '\0', 10, &gram)) {
                LOG_ERR("parse_u32() fail, str: %s", azArg[i]);
                goto out_fail;
            }
            if (gram < MIN_GRAM || gram > MAX_GRAM) {
                LOG_ERR("%u-gram is out of range, should in range [%d, %d]", gram, MIN_GRAM, MAX_GRAM);
                goto out_fail;
            }
            tok->ngram = gram;
        } else {
            LOG_ERR("unrecognizable option at index %d: %s", i, azArg[i]);
            goto out_fail;
        }
    }

    LOG_DBG("n-gram = %u", tok->ngram);
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
    LOG_DBG("Freeing FTS5 " LIBNAME " tokenizer...");

    assert_nonnull(pTok);
    ngram_tokenizer_t *tok = (ngram_tokenizer_t *) pTok;
    LOG_DBG("pTok: %p ngram: %u", tok, tok->ngram);

    sqlite3_free(tok);
}

typedef enum {
    DIGIT,
    SPACE_OR_CONTROL,
    ALPHABETIC,
    OTHER
} token_category_t;

static token_category_t get_token_category(char c) {
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

typedef int (*xTokenCallback)(
        void *pCtx,         /* Copy of 2nd argument to xTokenize() */
        int tflags,         /* Mask of FTS5_TOKEN_* flags */
        const char *pToken, /* Pointer to buffer containing token */
        int nToken,         /* Size of token in bytes */
        int iStart,         /* Byte offset of token within input text */
        int iEnd            /* Byte offset of end of token within input text */
);

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
    assert_nonnull(pTok);
    assert_nonnull(pCtx);
    assert_nonnull(pText);
    assert_ge(nText, 0, %d);
    assert_nonnull(xToken);

    fts5_api *pFts5Api = (fts5_api *) pCtx;
    UNUSED(pFts5Api);

    ngram_tokenizer_t *tok = (ngram_tokenizer_t *) pTok;
    LOG_DBG("%u-gram tokenizing...", tok->ngram);
    LOG_DBG("pTok: %p pCtx: %p flags: %#x", pTok, pCtx, flags);
    // [quote] ... pText may or may not be nul-terminated.
    LOG_DBG("nText: %d pText: %.*s", nText, nText, pText);
    LOG_DBG("xToken: %p", xToken);

    int iStart = 0;
    int iEnd = 0;
    int nthToken = 0;
    while (iEnd < nText) {
        u32 gram = 0;

        while (gram < tok->ngram && iEnd < nText) {
            token_category_t category = get_token_category(pText[iEnd]);

            if (category == OTHER) {
                int len = utf8_char_count(pText[iEnd]);
                if (len <= 0) {
                    LOG_ERR("Met non-UTF8 character at index %d", iEnd);
                    return SQLITE_ERROR;
                }
                iEnd += len;
            } else {
                while (++iEnd < nText && get_token_category(pText[iEnd]) == category) {
                    // continue
                }
            }

            if (category != SPACE_OR_CONTROL) {
                gram++;
            }
        }

        if (gram != 0) {
            const char *token = &pText[iStart];
            int tokenLen = iEnd - iStart;

            nthToken++;
            LOG_DBG("Token#%d: len: %d str: %.*s", nthToken, tokenLen, tokenLen, token);

            int rc = xToken(pCtx, 0, token, tokenLen, iStart, iEnd);
            if (rc != SQLITE_OK) {
                return rc;
            }
        }

        iStart = iEnd;
    }

    if (iEnd != nText) {
        // Certainly not a valid UTF-8 string
        return SQLITE_ERROR;
    }
    return SQLITE_OK;
}

/**
 * SQLite loadable extension entry point
 * see:
 *  https://www.sqlite.org/loadext.html#programming_loadable_extensions
 *  https://sqlite.org/fts5.html#extending_fts5
 */
#ifdef _WIN32
__declspec(dllexport)
#endif

int sqlite3_ngramporter_init(
        sqlite3 *db,
        char **pzErrMsg,
        const sqlite3_api_routines *pApi) {
    assert_nonnull(db);
    assert_nonnull(pzErrMsg);
    assert_nonnull(pApi);

    // Initialize the global sqlite3_api variable.
    //  so all sqlite3_*() functions can be used.
    SQLITE_EXTENSION_INIT2(pApi)

    LOG("HEAD commit: %s", BUILD_HEAD_COMMIT);
    LOG("Built by %s at %s", BUILD_USER, BUILD_TIMESTAMP);
    LOG("SQLite3 compile-time version: %s", SQLITE_VERSION);
    LOG("SQLite3 run-time version: %s", sqlite3_libversion());

    fts5_api *pFts5Api = fts5_api_from_db(db);
    if (pFts5Api == NULL) {
        int err = sqlite3_errcode(db);
        assert_nonzero(err, %d);
        *pzErrMsg = sqlite3_mprintf("%s: %d %s", __func__, err, sqlite3_errstr(err));
        return err;
    }
    assert_eq(pFts5Api->iVersion, 2, %d);

    fts5_tokenizer t = {
            .xCreate = ngram_create,
            .xDelete = ngram_delete,
            .xTokenize = ngram_tokenize,
    };
    return pFts5Api->xCreateTokenizer(pFts5Api, LIBNAME, (void *) pFts5Api, &t, NULL);
}
