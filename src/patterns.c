/* patterns.c — find links that live inside string literals.
 *
 * A route, an html id and a css class are not "code" to a compiler, they are
 * text. So instead of a grammar we search for a known prefix and read the
 * quoted string after it. Everything here expects an already comment-masked
 * copy (see mask.h), which is why a comment that mentions /api/scan no longer
 * shows up as a route, and $("tree") finally shows up as a DOM link.
 */
#include "patterns.h"
#include "model.h"
#include "store.h"
#include <ctype.h>
#include <string.h>

#define IDC(c) (isalnum((unsigned char)(c)) || (c) == '_' || (c) == '$')

/* Line numbers for a pointer into the text (pointers must come in increasing order). */
typedef struct { const char *at; int line; } LineCtr;
static int lc_line(LineCtr *c, const char *p) {
    for (; c->at < p; c->at++) if (*c->at == '\n') c->line++;
    return c->line;
}

/* If a quoted string starts at p (after optional spaces), give its content. */
static int quoted_at(const char *p, const char **start, size_t *len) {
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;
    char q = *p;
    if (q != '"' && q != '\'' && q != '`') return 0;
    const char *s = ++p;
    while (*p && *p != q && *p != '\n') p++;
    if (*p != q) return 0;
    *start = s;
    *len = (size_t)(p - s);
    return 1;
}

/* "div.row:hover" -> 3. Selectors are compound; we only keep the first term. */
static size_t first_term(const char *s, size_t n) {
    size_t i = 0;
    while (i < n && !strchr(" \t>+~,:#.[(", s[i])) i++;
    return i ? i : n;
}

static void add_ref_file(int f, int line, const char *s, size_t n, int kind) {
    if (n == 0) return;
    store_add_ref(f, -1, kind, s, n, line);
}

enum { P_ID, P_SELECTOR, P_QUERY, P_CLASSLIST, P_IMPORT };

static void scan_pattern(int f, const char *src, const char *needle, int mode) {
    LineCtr lc = { src, 1 };
    size_t nl = strlen(needle);
    for (const char *p = src; (p = strstr(p, needle)) != NULL; p += nl) {
        const char *q = p + nl, *s;
        size_t n;
        if (mode == P_CLASSLIST) {                 /* classList.add("x") */
            while (IDC(*q)) q++;
            if (*q != '(') continue;
            q++;
        }
        if (!quoted_at(q, &s, &n) || n == 0) continue;
        int line = lc_line(&lc, p);
        if (mode == P_ID) {
            add_ref_file(f, line, s, n, R_DOM);              /* getElementById("tree") */
        } else if (mode == P_SELECTOR || mode == P_QUERY) {
            /* $("tree"), $("#tree"), $(".card"), querySelector("#a .b") */
            if (*s == '#') add_ref_file(f, line, s + 1, first_term(s + 1, n - 1), R_DOM);
            else if (*s == '.') add_ref_file(f, line, s + 1, first_term(s + 1, n - 1), R_CLASS);
            else if (mode == P_SELECTOR) add_ref_file(f, line, s, first_term(s, n), R_DOM);
        } else if (mode == P_CLASSLIST) {
            add_ref_file(f, line, s, n, R_CLASS);
        } else if (mode == P_IMPORT) {             /* only relative paths: "./x.js" */
            if (*s == '.') add_ref_file(f, line, s, n, R_IMPORT);
        }
    }
}

void scan_dom_refs(int f, const char *src) {
    scan_pattern(f, src, "getElementById(", P_ID);
    scan_pattern(f, src, "$(", P_SELECTOR);       /* this project's shorthand helper */
    scan_pattern(f, src, "querySelectorAll(", P_QUERY);
    scan_pattern(f, src, "querySelector(", P_QUERY);
    scan_pattern(f, src, "classList.", P_CLASSLIST);
    scan_pattern(f, src, "require(", P_IMPORT);
    scan_pattern(f, src, "import(", P_IMPORT);
    scan_pattern(f, src, "import ", P_IMPORT);
    scan_pattern(f, src, " from ", P_IMPORT);
}

/* #include "local.h" (angle-bracket system headers are ignored) */
void scan_includes(int f, const char *src) {
    const char *p = src;
    int line = 1;
    while (*p) {
        const char *e = strchr(p, '\n');
        const char *end = e ? e : p + strlen(p);
        const char *q = p;
        while (q < end && (*q == ' ' || *q == '\t')) q++;
        if (q < end && *q == '#') {
            q++;
            while (q < end && (*q == ' ' || *q == '\t')) q++;
            if (end - q > 7 && !strncmp(q, "include", 7)) {
                q += 7;
                while (q < end && (*q == ' ' || *q == '\t')) q++;
                if (q < end && *q == '"') {
                    const char *s = ++q;
                    while (q < end && *q != '"') q++;
                    if (q < end) add_ref_file(f, line, s, (size_t)(q - s), R_INCLUDE);
                }
            }
        }
        if (!e) break;
        p = e + 1;
        line++;
    }
}

void scan_routes(int f, const char *src, int as_definition) {
    LineCtr lc = { src, 1 };
    for (const char *p = src; (p = strstr(p, "/api/")) != NULL; p += 5) {
        if (p == src) continue;
        char q = p[-1];
        if (q != '"' && q != '\'' && q != '`') continue;   /* must open a string */
        const char *e = p + 5;
        while (IDC(*e) || *e == '/' || *e == '-') e++;
        size_t n = (size_t)(e - p);
        if (n <= 5) continue;
        int line = lc_line(&lc, p);
        if (as_definition) store_add_sym(f, S_ROUTE, p, n, line);
        else store_add_ref(f, -1, R_API, p, n, line);
    }
}
