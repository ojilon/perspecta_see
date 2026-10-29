/* buf.c — the growable string buffer. Keep it boring: append, appendf, escape. */
#include "buf.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void buf_init(Buf *b) { b->p = NULL; b->len = 0; b->cap = 0; }
void buf_free(Buf *b) { free(b->p); buf_init(b); }

static int buf_reserve(Buf *b, size_t extra) {
    size_t need = b->len + extra + 1;
    if (need <= b->cap) return 1;
    size_t cap = b->cap ? b->cap : 1024;
    while (cap < need) cap *= 2;
    char *np = (char *)realloc(b->p, cap);
    if (!np) return 0;
    b->p = np;
    b->cap = cap;
    return 1;
}

void buf_addn(Buf *b, const char *s, size_t n) {
    if (!buf_reserve(b, n)) return;
    memcpy(b->p + b->len, s, n);
    b->len += n;
    b->p[b->len] = 0;
}

void buf_add(Buf *b, const char *s) { buf_addn(b, s, strlen(s)); }

void buf_addf(Buf *b, const char *fmt, ...) {
    char tmp[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if ((size_t)n < sizeof tmp) { buf_addn(b, tmp, (size_t)n); return; }
    /* too long for tmp: format again, straight into the buffer */
    if (!buf_reserve(b, (size_t)n)) return;
    va_start(ap, fmt);
    vsnprintf(b->p + b->len, (size_t)n + 1, fmt, ap);
    va_end(ap);
    b->len += (size_t)n;
}

void buf_json_str(Buf *b, const char *s) {
    buf_addn(b, "\"", 1);
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        switch (c) {
        case '"':  buf_add(b, "\\\""); break;
        case '\\': buf_add(b, "\\\\"); break;
        case '\n': buf_add(b, "\\n"); break;
        case '\r': buf_add(b, "\\r"); break;
        case '\t': buf_add(b, "\\t"); break;
        default:
            if (c < 0x20) buf_addf(b, "\\u%04x", (unsigned)c);
            else buf_addn(b, (const char *)&c, 1);
        }
    }
    buf_addn(b, "\"", 1);
}
