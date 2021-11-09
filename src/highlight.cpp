#include <cstring>

#include "highlight.h"
#include "utils.h"

SQLITE_EXTENSION_INIT3

// see:
//  https://github.com/sqlite/sqlite/blob/master/ext/fts5/fts5_aux.c
//  cad760d16ed403a065dbc90dd5c50f1eb29f5988

typedef struct CInstIter CInstIter;
struct CInstIter {
    const Fts5ExtensionApi *pApi;   /* API offered by current FTS version */
    Fts5Context *pFts;              /* First arg to pass to pApi functions */
    int iCol;                       /* Column to search */
    int iInst;                      /* Next phrase instance index */
    int nInst;                      /* Total number of phrase instances */

    /* Output variables */
    int iStart;                     /* First token in coalesced phrase instance */
    int iEnd;                       /* Last token in coalesced phrase instance */
};

/*
** Advance the iterator to the next coalesced phrase instance. Return
** an SQLite error code if an error occurs, or SQLITE_OK otherwise.
*/
static inline int fts5CInstIterNext(CInstIter *pIter) {
    int rc = SQLITE_OK;
    pIter->iStart = -1;
    pIter->iEnd = -1;

    while (rc == SQLITE_OK && pIter->iInst < pIter->nInst) {
        int ip;
        int ic;
        int io;
        rc = pIter->pApi->xInst(pIter->pFts, pIter->iInst, &ip, &ic, &io);
        if (rc == SQLITE_OK) {
            if (ic == pIter->iCol) {
                int iEnd = io - 1 + pIter->pApi->xPhraseSize(pIter->pFts, ip);
                if (pIter->iStart < 0) {
                    pIter->iStart = io;
                    pIter->iEnd = iEnd;
                } else if (io <= pIter->iEnd) {
                    if (iEnd > pIter->iEnd) pIter->iEnd = iEnd;
                } else {
                    break;
                }
            }
            pIter->iInst++;
        }
    }

    return rc;
}

/*
** Initialize the iterator object indicated by the final parameter to
** iterate through coalesced phrase instances in column iCol.
*/
static inline int fts5CInstIterInit(
        const Fts5ExtensionApi *pApi,
        Fts5Context *pFts,
        int iCol,
        CInstIter *pIter
) {
    int rc;

    memset(pIter, 0, sizeof(CInstIter));
    pIter->pApi = pApi;
    pIter->pFts = pFts;
    pIter->iCol = iCol;
    rc = pApi->xInstCount(pFts, &pIter->nInst);

    if (rc == SQLITE_OK) {
        rc = fts5CInstIterNext(pIter);
    }

    return rc;
}

// TODO: TBD

void ngram_highlight(
        const Fts5ExtensionApi *pApi,   /* API offered by current FTS version */
        Fts5Context *pFts,              /* First arg to pass to pApi functions */
        sqlite3_context *pCtx,          /* Context for returning result/error */
        int nVal,                       /* Number of values in apVal[] array */
        sqlite3_value **apVal           /* Array of trailing arguments */
) {
    UNUSED(pApi, pFts, pCtx, nVal, apVal);
}
