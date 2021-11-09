#include "highlight.h"

#include "utils.h"

void ngram_highlight(
        const Fts5ExtensionApi *pApi,   /* API offered by current FTS version */
        Fts5Context *pFts,              /* First arg to pass to pApi functions */
        sqlite3_context *pCtx,          /* Context for returning result/error */
        int nVal,                       /* Number of values in apVal[] array */
        sqlite3_value **apVal           /* Array of trailing arguments */
) {
    UNUSED(pApi, pFts, pCtx, nVal, apVal);
}
