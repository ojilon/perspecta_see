#ifndef BUF_H
#define BUF_H
#include <stddef.h>

/* buf.h — a growable text buffer.
 *
 * Every response the server builds (JSON for the graph, file contents) is
 * assembled here, so no caller has to guess a size up front. */

typedef struct { char *p; size_t len, cap; } Buf;

void buf_init(Buf *b);
void buf_free(Buf *b);
void buf_addn(Buf *b, const char *s, size_t n);
void buf_add(Buf *b, const char *s);
void buf_addf(Buf *b, const char *fmt, ...);
void buf_json_str(Buf *b, const char *s);   /* adds "..." with JSON escaping */

#endif
