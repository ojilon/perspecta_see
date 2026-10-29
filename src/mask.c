/* mask.c — blank out comments / strings while keeping line numbers.
 *
 * The parser has no real grammar. Instead it does what a human does when
 * skimming: first cross out the parts that are not code, then look for shapes
 * in what is left. Keeping the text the same length (comment bytes become
 * spaces, newlines survive) means every offset we found still points at the
 * same place in the browser's copy of the file.
 */
#include "mask.h"
#include "model.h"

/* Skip over a quoted literal starting at `i` (s[i] is the quote).
   Returns the index just past the closing quote. */
static size_t skip_string(const char *s, size_t n, size_t i) {
    char q = s[i++];
    while (i < n && s[i] != q) {
        if (s[i] == '\\' && i + 1 < n) i++;     /* \" does not end the string */
        i++;
    }
    return i < n ? i + 1 : n;
}

void mask_comments(char *s, size_t n, int lang) {
    size_t i = 0;
    while (i < n) {
        char c = s[i];
        if (c == '"' || c == '\'' || (lang == L_JS && c == '`')) {
            i = skip_string(s, n, i);           /* strings stay intact */
            continue;
        }
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
        i++;
    }
}

void mask_code(char *s, size_t n, int lang) {
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
        if (c == '"' || c == '\'' || (lang == L_JS && c == '`')) {
            size_t end = skip_string(s, n, i);
            for (size_t k = i + 1; k + 1 < end; k++) if (s[k] != '\n') s[k] = ' ';
            i = end;                            /* the quotes themselves stay */
            continue;
        }
        i++;
    }
}

void mask_preproc(char *s, size_t n) {
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
            int cont = (e > p && s[e - 1] == '\\');       /* continuation line */
            for (size_t q = p; q < e; q++) s[q] = ' ';
            in_pp = cont;
        }
        p = e < n ? e + 1 : n;
    }
}
