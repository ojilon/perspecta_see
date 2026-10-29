/* parse.c — choose the pipeline for one file.
 *
 * The order matters, and it is the same idea for every language:
 *
 *   1. comment-masked copy  -> raw-text scans (routes, ids, includes, imports).
 *      Strings must still be readable here: a route *is* a string.
 *   2. fully masked copy    -> the code walker (definitions, scopes, calls).
 *      Strings must be gone here: `if (c == "{")` must not open a block.
 *   3. tie every loose reference to the function that contains it, so the
 *      browser can draw "this function uses that html id" lines.
 *
 * Every step keeps the byte count and the newlines, so all of them agree on
 * what "line 42" means.
 */
#include "parse.h"
#include "mask.h"
#include "model.h"
#include "parse_code.h"
#include "parse_css.h"
#include "parse_html.h"
#include "patterns.h"
#include "store.h"
#include <stdlib.h>
#include <string.h>

static char *dup_text(const char *src, size_t n) {
    char *m = (char *)malloc(n + 1);
    if (!m) return NULL;
    memcpy(m, src, n);
    m[n] = 0;
    return m;
}

void parse_js_text(int f, const char *src, size_t n) {
    char *mc = dup_text(src, n);
    if (mc) {
        mask_comments(mc, n, L_JS);          /* strings stay: $("tree"), "/api/x" */
        scan_dom_refs(f, mc);
        scan_routes(f, mc, 0);
        free(mc);
    }
    char *m = dup_text(src, n);
    if (m) {
        mask_code(m, n, L_JS);               /* strings go: they are not code */
        scan_code(f, L_JS, m, n);
        free(m);
    }
}

void parse_css_text(int f, const char *src, size_t n) {
    char *m = dup_text(src, n);
    if (!m) return;
    mask_code(m, n, L_CSS);
    scan_css(f, m, n);
    free(m);
}

static void parse_c_like(int f, const char *src, size_t n, int lang) {
    char *mc = dup_text(src, n);
    if (mc) {
        mask_comments(mc, n, lang);
        scan_includes(f, mc);                /* #include "local.h" */
        scan_routes(f, mc, 1);               /* "/api/..." in C = a route definition */
        free(mc);
    }
    char *m = dup_text(src, n);
    if (m) {
        mask_code(m, n, lang);
        mask_preproc(m, n);                  /* a macro body is not a function */
        scan_code(f, lang, m, n);
        free(m);
    }
}

void parse_file(int f, const char *src, size_t n) {
    int lang = g_files[f].lang;
    int ref_mark = store_ref_mark();
    store_begin_file();

    switch (lang) {
    case L_HTML: parse_html(f, src, n); break;
    case L_CSS:  parse_css_text(f, src, n); break;
    case L_JS:   parse_js_text(f, src, n); break;
    case L_C: case L_H: parse_c_like(f, src, n, lang); break;
    default:     return;
    }

    /* Loose references (found by the string scans) now learn which function
       they sit in. This is what turns "this file mentions #tree" into
       "drawTree() uses #tree". */
    for (int r = ref_mark; r < g_nrefs; r++)
        if (g_refs[r].from_sym < 0) g_refs[r].from_sym = store_enclosing(g_refs[r].line);
}
