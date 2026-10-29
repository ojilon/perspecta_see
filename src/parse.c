/* parse.c: "digest" one file. No real grammar: we blank out comments and
   strings, then walk the text looking for patterns. Good enough to start,
   and every rule here is a place you can improve later. */
#include "parse.h"
#include "model.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define IDS(c) (isalpha((unsigned char)(c)) || (c) == '_' || (c) == '$')
#define IDC(c) (isalnum((unsigned char)(c)) || (c) == '_' || (c) == '$')

static int g_first_sym;   /* index of the first symbol of the file being parsed */

/* ---------- recording ---------- */
static int add_sym(int f, int kind, const char *name, size_t nl, int line) {
    if (nl > 63) nl = 63;
    if (kind != S_FUNC)   /* ids, classes, routes: keep one entry per name per file */
        for (int i = g_first_sym; i < g_nsyms; i++)
            if (g_syms[i].kind == kind && strlen(g_syms[i].name) == nl &&
                !strncmp(g_syms[i].name, name, nl))
                return i;
    if (g_nsyms >= MAX_SYMS) return -1;
    Sym *s = &g_syms[g_nsyms];
    s->file = f; s->kind = kind; s->line = line; s->end = line;
    memcpy(s->name, name, nl);
    s->name[nl] = 0;
    return g_nsyms++;
}

static void add_ref(int f, int from_sym, int kind, const char *t, size_t tl, int line) {
    if (g_nrefs >= MAX_REFS) return;
    if (tl > 127) tl = 127;
    Ref *r = &g_refs[g_nrefs++];
    r->file = f; r->from_sym = from_sym; r->kind = kind; r->line = line;
    memcpy(r->target, t, tl);
    r->target[tl] = 0;
}

/* Line numbers for a pointer into the text (pointers must come in increasing order). */
typedef struct { const char *at; int line; } LineCtr;
static int lc_line(LineCtr *c, const char *p) {
    for (; c->at < p; c->at++) if (*c->at == '\n') c->line++;
    return c->line;
}

/* ---------- step 1: blank out comments and strings ---------- */
/* Same length, same newlines, so offsets and line numbers stay valid. */
static void mask_code(char *s, size_t n, int lang) {
    size_t i = 0;
    while (i < n) {
        char c = s[i];
        if (c == '/' && i + 1 < n && s[i + 1] == '/' && lang != L_CSS) {
            while (i < n && s[i] != '\n') s[i++] = ' ';
            continue;
        }
        if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            s[i] = s[i + 1] = ' ';
            i += 2;
            while (i < n && !(s[i] == '*' && i + 1 < n && s[i + 1] == '/')) {
                if (s[i] != '\n') s[i] = ' ';
                i++;
            }
            if (i < n) { s[i] = ' '; if (i + 1 < n) s[i + 1] = ' '; i += 2; }
            continue;
        }
        if (lang != L_CSS && (c == '"' || c == '\'' || (lang == L_JS && c == '`'))) {
            char q = c;
            i++;
            while (i < n && s[i] != q) {
                if (s[i] == '\\' && i + 1 < n) { s[i] = ' '; i++; }
                if (s[i] != '\n') s[i] = ' ';
                i++;
            }
            if (i < n) i++;
            continue;
        }
        i++;
    }
}

/* C only: blank whole preprocessor lines (#define, #include, ...), including continuations. */
static void mask_preproc(char *s, size_t n) {
    size_t p = 0;
    int in_pp = 0;
    while (p < n) {
        size_t e = p;
        while (e < n && s[e] != '\n') e++;
        if (!in_pp) {
            size_t j = p;
            while (j < e && (s[j] == ' ' || s[j] == '\t')) j++;
            if (j < e && s[j] == '#') in_pp = 1;
        }
        if (in_pp) {
            int cont = (e > p && s[e - 1] == '\\');
            for (size_t q = p; q < e; q++) s[q] = ' ';
            in_pp = cont;
        }
        p = e < n ? e + 1 : n;
    }
}

/* ---------- step 2a: raw-text patterns (strings still intact) ---------- */
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

enum { P_ID, P_QUERY, P_CLASSLIST, P_IMPORT };

static void scan_pattern(int f, const char *src, const char *needle, int mode) {
    LineCtr lc = { src, 1 };
    size_t nl = strlen(needle);
    for (const char *p = src; (p = strstr(p, needle)) != NULL; p += nl) {
        const char *q = p + nl, *s;
        size_t n;
        if (mode == P_CLASSLIST) {          /* classList.add( "x" ) */
            while (IDC(*q)) q++;
            if (*q != '(') continue;
            q++;
        }
        if (!quoted_at(q, &s, &n) || n == 0) continue;
        int line = lc_line(&lc, p);
        if (mode == P_ID) add_ref(f, -1, R_DOM, s, n, line);
        else if (mode == P_CLASSLIST) add_ref(f, -1, R_CLASS, s, n, line);
        else if (mode == P_QUERY) {         /* querySelector("#id") or (".class") */
            if (*s == '#') add_ref(f, -1, R_DOM, s + 1, n - 1, line);
            else if (*s == '.') add_ref(f, -1, R_CLASS, s + 1, n - 1, line);
        } else if (mode == P_IMPORT) {      /* only relative paths: "./x.js" */
            if (*s == '.') add_ref(f, -1, R_IMPORT, s, n, line);
        }
    }
}

/* #include "local.h" (angle-bracket system headers are ignored) */
static void scan_includes(int f, const char *src) {
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
                    if (q < end) add_ref(f, -1, R_INCLUDE, s, (size_t)(q - s), line);
                }
            }
        }
        if (!e) break;
        p = e + 1;
        line++;
    }
}

/* "/api/something": defined in C (a route), used in JS (a fetch). This is the
   bridge between frontend and backend. */
static void scan_api(int f, const char *src, int in_c) {
    LineCtr lc = { src, 1 };
    for (const char *p = src; (p = strstr(p, "/api/")) != NULL; p += 5) {
        if (p == src) continue;
        char q = p[-1];
        if (q != '"' && q != '\'' && q != '`') continue;
        const char *e = p + 5;
        while (IDC(*e) || *e == '/' || *e == '-') e++;
        size_t n = (size_t)(e - p);
        if (n <= 5) continue;
        int line = lc_line(&lc, p);
        if (in_c) add_sym(f, S_ROUTE, p, n, line);
        else add_ref(f, -1, R_API, p, n, line);
    }
}

/* ---------- step 2b: C and JS: functions and calls ---------- */
static const char *KW[] = { "if", "for", "while", "switch", "return", "sizeof", "catch",
    "function", "typeof", "else", "do", "case", "defined", "new", "await", "delete",
    "void", "throw", "in", "of", "instanceof", NULL };
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

static void scan_code(int f, int lang, const char *m, size_t n) {
    int js = (lang == L_JS), line = 1, depth = 0, cur = -1;
    size_t i = 0;
    while (i < n) {
        char c = m[i];
        if (c == '\n') { line++; i++; continue; }
        if (c == '{') { depth++; i++; continue; }
        if (c == '}') {
            if (depth > 0) depth--;
            if (depth == 0 && cur >= 0) { g_syms[cur].end = line; cur = -1; }
            i++;
            continue;
        }
        if (c == ';' && depth == 0) {
            if (cur >= 0) { g_syms[cur].end = line; cur = -1; }
            i++;
            continue;
        }
        if (!IDS(c)) { i++; continue; }

        size_t s = i;
        while (i < n && IDC(m[i])) i++;
        size_t wl = i - s, cl = wl > 63 ? 63 : wl;
        char w[64];
        memcpy(w, m + s, cl);
        w[cl] = 0;

        if (js && depth == 0) {
            size_t j = i;
            if (!strcmp(w, "function")) {                 /* function name(...) */
                while (j < n && (m[j] == ' ' || m[j] == '\t' || m[j] == '*')) j++;
                if (j < n && IDS(m[j])) {
                    size_t ns = j;
                    while (j < n && IDC(m[j])) j++;
                    cur = add_sym(f, S_FUNC, m + ns, j - ns, line);
                    i = j;
                }
                continue;
            }
            if (!strcmp(w, "const") || !strcmp(w, "let") || !strcmp(w, "var")) {
                skip_sp(m, n, &j, 0);                     /* const name = (...) => ... */
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
                            if (k) { k++; skip_sp(m, n, &k, 1); if (m[k] == '=' && m[k + 1] == '>') fn = 1; }
                        } else if (IDS(m[j])) {
                            while (IDC(m[j])) j++;
                            skip_sp(m, n, &j, 0);
                            if (m[j] == '=' && m[j + 1] == '>') fn = 1;
                        }
                        if (fn) { cur = add_sym(f, S_FUNC, m + ns, ne - ns, line); i = ne; }
                    }
                }
                continue;
            }
        }

        size_t j = i;
        skip_sp(m, n, &j, 0);
        if (j >= n || m[j] != '(' || is_kw(w)) continue;

        if (!js && depth == 0) {                          /* C: name(...) { at top level = a definition */
            size_t k = match_paren(m, n, j);
            if (k) {
                k++;
                skip_sp(m, n, &k, 1);
                if (k < n && m[k] == '{') cur = add_sym(f, S_FUNC, w, cl, line);
            }
        } else if (depth > 0 && cur >= 0) {               /* inside a function body: a call */
            size_t b = s;
            while (b > 0 && (m[b - 1] == ' ' || m[b - 1] == '\t')) b--;
            int member = (b > 0 && m[b - 1] == '.') || (b > 1 && m[b - 1] == '>' && m[b - 2] == '-');
            if (!member) add_ref(f, cur, R_CALL, w, cl, line);
        }
    }
}

/* Which function contains this line? (for refs found by raw-text scans) */
static int enclosing(int line) {
    for (int i = g_first_sym; i < g_nsyms; i++)
        if (g_syms[i].kind == S_FUNC && line >= g_syms[i].line && line <= g_syms[i].end) return i;
    return -1;
}

/* ---------- step 2c: HTML ---------- */
static int ci_starts(const char *s, const char *pre) {   /* pre must be lowercase */
    for (; *pre; s++, pre++) if (tolower((unsigned char)*s) != *pre) return 0;
    return 1;
}

static int ends_with(const char *s, const char *suf) {
    size_t a = strlen(s), b = strlen(suf);
    return a >= b && !strcmp(s + a - b, suf);
}

static void scan_html(int f, const char *src, size_t n) {
    size_t i = 0;
    int line = 1;
    while (i < n) {
        if (src[i] == '\n') { line++; i++; continue; }
        if (src[i] != '<') { i++; continue; }
        if (!strncmp(src + i, "<!--", 4)) {
            i += 4;
            while (i < n && strncmp(src + i, "-->", 3)) { if (src[i] == '\n') line++; i++; }
            i = i < n ? i + 3 : n;
            continue;
        }
        size_t j = i + 1;
        if (j < n && src[j] == '/') { i = j; continue; }
        char tag[16];
        size_t tl = 0;
        while (j < n && (isalnum((unsigned char)src[j]) || src[j] == '-') && tl < 15)
            tag[tl++] = (char)tolower((unsigned char)src[j++]);
        tag[tl] = 0;
        if (!tl) { i++; continue; }

        int tline = line;
        char a_src[200] = "", a_href[200] = "", a_id[64] = "", a_class[200] = "";
        while (j < n && src[j] != '>') {                  /* attributes: name="value" */
            unsigned char ch = (unsigned char)src[j];
            if (ch == '\n') { line++; j++; continue; }
            if (isspace(ch) || ch == '/') { j++; continue; }
            char an[24];
            size_t al = 0;
            while (j < n && !isspace((unsigned char)src[j]) && src[j] != '=' && src[j] != '>' && src[j] != '/') {
                if (al < 23) an[al++] = (char)tolower((unsigned char)src[j]);
                j++;
            }
            an[al] = 0;
            while (j < n && (src[j] == ' ' || src[j] == '\t')) j++;
            if (j < n && src[j] == '=') {
                j++;
                while (j < n && isspace((unsigned char)src[j])) j++;
                const char *vs;
                size_t vl;
                if (j < n && (src[j] == '"' || src[j] == '\'')) {
                    char q = src[j++];
                    vs = src + j;
                    while (j < n && src[j] != q) { if (src[j] == '\n') line++; j++; }
                    vl = (size_t)(src + j - vs);
                    if (j < n) j++;
                } else {
                    vs = src + j;
                    while (j < n && !isspace((unsigned char)src[j]) && src[j] != '>') j++;
                    vl = (size_t)(src + j - vs);
                }
                char *dst = NULL;
                size_t cap = 0;
                if (!strcmp(an, "src")) { dst = a_src; cap = sizeof a_src; }
                else if (!strcmp(an, "href")) { dst = a_href; cap = sizeof a_href; }
                else if (!strcmp(an, "id")) { dst = a_id; cap = sizeof a_id; }
                else if (!strcmp(an, "class")) { dst = a_class; cap = sizeof a_class; }
                if (dst) { if (vl >= cap) vl = cap - 1; memcpy(dst, vs, vl); dst[vl] = 0; }
            }
        }

        if (a_id[0]) add_sym(f, S_ID, a_id, strlen(a_id), tline);
        if (!strcmp(tag, "script") && a_src[0] && !strstr(a_src, "//"))
            add_ref(f, -1, R_SCRIPT, a_src, strlen(a_src), tline);
        if (!strcmp(tag, "link") && ends_with(a_href, ".css") && !strstr(a_href, "//"))
            add_ref(f, -1, R_STYLE, a_href, strlen(a_href), tline);
        for (char *t = a_class; *t;) {
            while (*t == ' ' || *t == '\t' || *t == '\n') t++;
            char *e = t;
            while (*e && *e != ' ' && *e != '\t' && *e != '\n') e++;
            if (e > t) add_ref(f, -1, R_CLASS, t, (size_t)(e - t), tline);
            t = e;
        }

        i = j < n ? j + 1 : n;
        if (!strcmp(tag, "script") || !strcmp(tag, "style")) {   /* skip raw text (inline code: not parsed yet) */
            const char *close = !strcmp(tag, "script") ? "</script" : "</style";
            while (i < n && !(src[i] == '<' && ci_starts(src + i, close))) {
                if (src[i] == '\n') line++;
                i++;
            }
        }
    }
}

/* ---------- step 2d: CSS ---------- */
static void scan_css(int f, const char *m, size_t n) {
    int line = 1, sp = 0, at = 0, seen = 0, br = 0;
    char st[32];                 /* block stack: 1 = @media-like group, 2 = declarations */
    size_t pre = 0;              /* where the current "prelude" (selector or @rule) began */
    for (size_t i = 0; i < n; i++) {
        char c = m[i];
        if (c == '\n') line++;
        if (!seen && !isspace((unsigned char)c) && c != '{' && c != '}' && c != ';') { seen = 1; at = (c == '@'); }
        if (c == '{') { if (sp < 31) st[sp++] = at ? 1 : 2; pre = i + 1; seen = 0; at = 0; br = 0; }
        else if (c == '}') { if (sp > 0) sp--; pre = i + 1; seen = 0; at = 0; br = 0; }
        else if (c == ';') {
            if (at) {                                       /* @import "x.css"; */
                const char *a = m + pre, *e = m + i, *q = a;
                while (q < e && *q != '"' && *q != '\'') q++;
                if (q < e) {
                    char qc = *q++;
                    const char *s = q;
                    while (q < e && *q != qc) q++;
                    add_ref(f, -1, R_IMPORT, s, (size_t)(q - s), line);
                }
            }
            pre = i + 1; seen = 0; at = 0;
        }
        else if (c == '[') br++;
        else if (c == ']') { if (br > 0) br--; }
        else if ((c == '.' || c == '#') && !at && br == 0 && (sp == 0 || st[sp - 1] == 1) && i + 1 < n &&
                 (isalpha((unsigned char)m[i + 1]) || m[i + 1] == '_' || m[i + 1] == '-')) {
            size_t s = i + 1, e = s;
            while (e < n && (isalnum((unsigned char)m[e]) || m[e] == '_' || m[e] == '-')) e++;
            add_sym(f, c == '.' ? S_CLASS : S_ID, m + s, e - s, line);
            i = e - 1;
        }
    }
}

/* ---------- entry point ---------- */
void parse_file(int f, const char *src, size_t n) {
    int lang = g_files[f].lang;
    int first_ref = g_nrefs;
    g_first_sym = g_nsyms;

    if (lang == L_HTML) { scan_html(f, src, n); return; }

    char *m = (char *)malloc(n + 1);          /* masked copy: comments + strings blanked */
    if (!m) return;
    memcpy(m, src, n);
    m[n] = 0;

    if (lang == L_CSS) {
        mask_code(m, n, L_CSS);
        scan_css(f, m, n);
    } else if (lang == L_C || lang == L_H || lang == L_JS) {
        if (lang != L_JS) scan_includes(f, src);
        scan_api(f, src, lang != L_JS);
        if (lang == L_JS) {
            scan_pattern(f, src, "getElementById(", P_ID);
            scan_pattern(f, src, "querySelector(", P_QUERY);
            scan_pattern(f, src, "querySelectorAll(", P_QUERY);
            scan_pattern(f, src, "classList.", P_CLASSLIST);
            scan_pattern(f, src, "require(", P_IMPORT);
            scan_pattern(f, src, "import(", P_IMPORT);
            scan_pattern(f, src, "import ", P_IMPORT);
            scan_pattern(f, src, " from ", P_IMPORT);
        }
        mask_code(m, n, lang);
        if (lang != L_JS) mask_preproc(m, n);
        scan_code(f, lang, m, n);
        for (int r = first_ref; r < g_nrefs; r++)
            if (g_refs[r].from_sym < 0) g_refs[r].from_sym = enclosing(g_refs[r].line);
    }
    free(m);
}