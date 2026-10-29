/* parse_css.c — selectors and @import from a masked stylesheet.
 *
 * We only care about the two things that can be linked: class/id selectors and
 * imported files. A selector is recognised when a `.` or `#` starts a name and
 * we are not already inside a declaration block, which is why the walker keeps
 * a tiny stack: brace -> is it an at-rule group or a declarations block?
 */
#include "parse_css.h"
#include "model.h"
#include "store.h"
#include <ctype.h>
#include <string.h>

#define NAMEC(c) (isalnum((unsigned char)(c)) || (c) == '_' || (c) == '-')

void scan_css(int f, const char *m, unsigned long n) {
    int line = 1, sp = 0, at = 0, seen = 0, br = 0;
    char st[32];              /* block stack: 1 = @media-like group, 2 = declarations */
    unsigned long pre = 0;    /* where the current prelude (selector or @rule) began */

    for (unsigned long i = 0; i < n; i++) {
        char c = m[i];
        if (c == '\n') line++;
        if (!seen && !isspace((unsigned char)c) && c != '{' && c != '}' && c != ';') {
            seen = 1;
            at = (c == '@');
        }
        if (c == '{') {
            if (sp < 31) st[sp++] = at ? 1 : 2;
            pre = i + 1; seen = 0; at = 0; br = 0;
        } else if (c == '}') {
            if (sp > 0) sp--;
            pre = i + 1; seen = 0; at = 0; br = 0;
        } else if (c == ';') {
            if (at) {                                   /* @import "x.css"; */
                const char *a = m + pre, *e = m + i, *q = a;
                while (q < e && *q != '"' && *q != '\'') q++;
                if (q < e) {
                    char qc = *q++;
                    const char *s = q;
                    while (q < e && *q != qc) q++;
                    store_add_ref(f, -1, R_IMPORT, s, (size_t)(q - s), line);
                }
            }
            pre = i + 1; seen = 0; at = 0;
        } else if (c == '[') {
            br++;                                        /* attribute selector: skip it */
        } else if (c == ']') {
            if (br > 0) br--;
        } else if ((c == '.' || c == '#') && !at && br == 0 && (sp == 0 || st[sp - 1] == 1) &&
                   i + 1 < n && (isalpha((unsigned char)m[i + 1]) || m[i + 1] == '_' || m[i + 1] == '-')) {
            unsigned long s = i + 1, e = s;
            while (e < n && NAMEC(m[e])) e++;
            store_add_sym(f, c == '.' ? S_CLASS : S_ID, m + s, (size_t)(e - s), line);
            i = e - 1;
        }
    }
}
