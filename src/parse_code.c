/* parse_code.c — walk masked C/JS text and record functions, scopes and calls.
 *
 * The walker keeps a small stack of open bodies: every `{` pushes (depth,
 * symbol) and the matching `}` closes it, which is how a function gets its
 * line range. That range is what the browser uses to draw "function scope"
 * boxes and to scroll the inline editor straight to the body.
 *
 * Two heuristics are worth knowing about:
 *
 *   1. `name(...)` followed by `{` defines something. In C that only counts at
 *      top level; in JS it also counts inside a class or object literal, which
 *      is how `draw() { ... }` style methods are picked up.
 *   2. Inside a body, `name(` is a call; `obj.name(` is a member call and is
 *      recorded separately (R_MCALL), because a member name is much weaker
 *      evidence than a bare one.
 */
#include "parse_code.h"
#include "model.h"
#include "store.h"
#include <ctype.h>
#include <string.h>

#define IDS(c) (isalpha((unsigned char)(c)) || (c) == '_' || (c) == '$')
#define IDC(c) (isalnum((unsigned char)(c)) || (c) == '_' || (c) == '$')

/* Words that are followed by "(" but never name a function. */
static const char *KW[] = { "if", "for", "while", "switch", "return", "sizeof", "catch",
    "function", "typeof", "else", "do", "case", "defined", "new", "await", "delete",
    "void", "throw", "in", "of", "instanceof", "yield", "super", "with", NULL };

static int is_kw(const char *w) {
    for (int i = 0; KW[i]; i++) if (!strcmp(w, KW[i])) return 1;
    return 0;
}

/* m[j] == '(' : return the index of its matching ')' (0 if none). */
static size_t match_paren(const char *m, size_t n, size_t j) {
    int d = 0;
    for (size_t i = j; i < n; i++) {
        if (m[i] == '(') d++;
        else if (m[i] == ')' && --d == 0) return i;
    }
    return 0;
}

static void skip_sp(const char *m, size_t n, size_t *j, int nl) {
    while (*j < n && (m[*j] == ' ' || m[*j] == '\t' || (nl && (m[*j] == '\n' || m[*j] == '\r')))) (*j)++;
}

/* Open bodies. 64 is far past anything a heuristic parser should try to nest. */
typedef struct { int depth[64]; int sym[64]; int n; } Scopes;

void scan_code(int f, int lang, const char *m, size_t n) {
    int js = (lang == L_JS);
    int line = 1, depth = 0, pending = -1;      /* pending: symbol waiting for its '{' */
    Scopes sc;
    sc.n = 0;

    size_t i = 0;
    while (i < n) {
        char c = m[i];
        if (c == '\n') { line++; i++; continue; }

        if (c == '{') {
            depth++;
            if (pending >= 0 && sc.n < 64) {     /* this brace opens the pending body */
                sc.depth[sc.n] = depth;
                sc.sym[sc.n++] = pending;
                pending = -1;
            }
            i++;
            continue;
        }
        if (c == '}') {
            if (sc.n > 0 && sc.depth[sc.n - 1] == depth) store_set_end(sc.sym[--sc.n], line);
            if (depth > 0) depth--;
            i++;
            continue;
        }
        if (c == ';') { pending = -1; i++; continue; }   /* "const f = () => x;" has no body */
        if (!IDS(c)) { i++; continue; }

        size_t s = i;
        while (i < n && IDC(m[i])) i++;
        size_t cl = i - s > 63 ? 63 : i - s;
        char w[64];
        memcpy(w, m + s, cl);
        w[cl] = 0;

        int cur = sc.n ? sc.sym[sc.n - 1] : -1;

        if (js && !strcmp(w, "function")) {              /* function name(...) */
            size_t j = i;
            while (j < n && (m[j] == ' ' || m[j] == '\t' || m[j] == '*')) j++;
            if (j < n && IDS(m[j])) {
                size_t ns = j;
                while (j < n && IDC(m[j])) j++;
                pending = store_add_sym(f, S_FUNC, m + ns, j - ns, line);
                while (j < n && m[j] == ' ') j++;
                if (j < n && m[j] == '(') {
                    size_t k = match_paren(m, n, j);
                    if (k) j = k + 1;
                }
                i = j;
            }
            continue;
        }
        if (js && depth == 0 &&
            (!strcmp(w, "const") || !strcmp(w, "let") || !strcmp(w, "var"))) {
            /* const name = (...) => ...   |   const name = function (...)   */
            size_t j = i;
            skip_sp(m, n, &j, 0);
            if (j < n && IDS(m[j])) {
                size_t ns = j;
                while (j < n && IDC(m[j])) j++;
                size_t ne = j;
                skip_sp(m, n, &j, 0);
                if (j + 1 < n && m[j] == '=' && m[j + 1] != '=' && m[j + 1] != '>') {
                    j++;
                    skip_sp(m, n, &j, 1);
                    if (!strncmp(m + j, "async", 5) && !IDC(m[j + 5])) { j += 5; skip_sp(m, n, &j, 1); }
                    int fn = 0;
                    if (!strncmp(m + j, "function", 8) && !IDC(m[j + 8])) fn = 1;
                    else if (m[j] == '(') {
                        size_t k = match_paren(m, n, j);
                        if (k) {
                            size_t a = k + 1;
                            skip_sp(m, n, &a, 1);
                            if (m[a] == '=' && m[a + 1] == '>') fn = 1;
                        }
                    } else if (IDS(m[j])) {
                        while (j < n && IDC(m[j])) j++;
                        skip_sp(m, n, &j, 0);
                        if (m[j] == '=' && m[j + 1] == '>') fn = 1;
                    }
                    if (fn) pending = store_add_sym(f, S_FUNC, m + ns, ne - ns, line);
                }
            }
            continue;
        }

        size_t j = i;
        skip_sp(m, n, &j, 0);
        if (j >= n || m[j] != '(' || is_kw(w)) continue;

        size_t k = match_paren(m, n, j);
        if (!k) continue;

        size_t b = s;                                     /* is it obj.name( ... )? */
        while (b > 0 && (m[b - 1] == ' ' || m[b - 1] == '\t')) b--;
        int member = (b > 0 && m[b - 1] == '.') ||
                     (b > 1 && m[b - 1] == '>' && m[b - 2] == '-');

        size_t after = k + 1;
        skip_sp(m, n, &after, 0);
        int is_def = (after < n && m[after] == '{');

        if (is_def && (js || (depth == 0 && !member))) {  /* a definition */
            pending = store_add_sym(f, S_FUNC, w, cl, line);
            i = k + 1;                                    /* skip the params, the loop hits '{' */
            continue;
        }
        if (cur >= 0) store_add_ref(f, cur, member ? R_MCALL : R_CALL, w, cl, line);
    }
}
