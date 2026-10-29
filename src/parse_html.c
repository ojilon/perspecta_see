/* parse_html.c — tags today, inline <script>/<style> blocks too.
 *
 * Two passes over the same text:
 *   1. walk the tags, record ids/classes and the files they pull in, and note
 *      where every inline <script> / <style> body starts and ends;
 *   2. for each inline body, build a copy of the whole file in which everything
 *      except that body is blanked (newlines kept), then run the normal JS or
 *      CSS parser on it.
 *
 * Pass 2 is what keeps line numbers honest: the JS parser sees a file of the
 * same length and shape, so "line 42" means line 42 of the html file.
 */
#include "parse_html.h"
#include "model.h"
#include "parse.h"
#include "store.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INLINE 64

typedef struct { size_t start, end; int lang; } Inline;

static int ci_starts(const char *s, const char *pre) {    /* pre must be lowercase */
    for (; *pre; s++, pre++) if (tolower((unsigned char)*s) != *pre) return 0;
    return 1;
}

static int ends_with(const char *s, const char *suf) {
    size_t a = strlen(s), b = strlen(suf);
    return a >= b && !strcmp(s + a - b, suf);
}

/* Blank everything except [start,end) but keep newlines, then parse it. */
static void parse_inline(int f, const char *src, size_t n, const Inline *in) {
    char *copy = (char *)malloc(n + 1);
    if (!copy) return;
    for (size_t i = 0; i < n; i++) copy[i] = (src[i] == '\n' || src[i] == '\r') ? src[i] : ' ';
    size_t len = in->end > in->start ? in->end - in->start : 0;
    memcpy(copy + in->start, src + in->start, len);
    copy[n] = 0;
    if (in->lang == L_CSS) parse_css_text(f, copy, n);
    else parse_js_text(f, copy, n);
    free(copy);
}

void parse_html(int f, const char *src, size_t n) {
    Inline spans[MAX_INLINE];
    int nspans = 0;

    size_t i = 0;
    int line = 1;
    while (i < n) {
        if (src[i] == '\n') { line++; i++; continue; }
        if (src[i] != '<') { i++; continue; }
        if (!strncmp(src + i, "<!--", 4)) {                     /* html comment */
            i += 4;
            while (i < n && strncmp(src + i, "-->", 3)) { if (src[i] == '\n') line++; i++; }
            i = i < n ? i + 3 : n;
            continue;
        }
        size_t j = i + 1;
        if (j < n && src[j] == '/') { i = j; continue; }         /* closing tag */
        char tag[16];
        size_t tl = 0;
        while (j < n && (isalnum((unsigned char)src[j]) || src[j] == '-') && tl < 15)
            tag[tl++] = (char)tolower((unsigned char)src[j++]);
        tag[tl] = 0;
        if (!tl) { i++; continue; }

        int tline = line;
        char a_src[200] = "", a_href[200] = "", a_id[64] = "", a_class[200] = "";
        while (j < n && src[j] != '>') {                         /* attributes: name="value" */
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

        if (a_id[0]) store_add_sym(f, S_ID, a_id, strlen(a_id), tline);
        int is_js = !strcmp(tag, "script");
        int is_css = !strcmp(tag, "style");
        if (is_js && a_src[0] && !strstr(a_src, "//"))
            store_add_ref(f, -1, R_SCRIPT, a_src, strlen(a_src), tline);
        if (!strcmp(tag, "link") && ends_with(a_href, ".css") && !strstr(a_href, "//"))
            store_add_ref(f, -1, R_STYLE, a_href, strlen(a_href), tline);
        if (!strcmp(tag, "a") && ends_with(a_href, ".html") && !strstr(a_href, "//") && a_href[0] != '#')
            store_add_ref(f, -1, R_HREF, a_href, strlen(a_href), tline);
        for (char *t = a_class; *t;) {                           /* class="a b c" */
            while (*t == ' ' || *t == '\t' || *t == '\n') t++;
            char *e = t;
            while (*e && *e != ' ' && *e != '\t' && *e != '\n') e++;
            if (e > t) store_add_ref(f, -1, R_CLASS, t, (size_t)(e - t), tline);
            t = e;
        }

        i = j < n ? j + 1 : n;
        if (is_js || is_css) {                                   /* raw text until </script|else> */
            const char *close = is_js ? "</script" : "</style";
            size_t body = i;
            while (i < n && !(src[i] == '<' && ci_starts(src + i, close))) {
                if (src[i] == '\n') line++;
                i++;
            }
            if (!a_src[0] && nspans < MAX_INLINE && i > body) {
                spans[nspans].start = body;
                spans[nspans].end = i;
                spans[nspans].lang = is_js ? L_JS : L_CSS;
                nspans++;
            }
        }
    }

    for (int k = 0; k < nspans; k++) parse_inline(f, src, n, &spans[k]);
}
