#ifndef BUF_H
#define BUF_H
#include <stddef.h>

/* A growable text buffer: lets us build big strings (JSON, file contents)
   without guessing the size up front. */
typedef struct { char *p; size_t len, cap; } Buf;

void buf_init(Buf *b);
void buf_free(Buf *b);
void buf_addn(Buf *b, const char *s, size_t n);
void buf_add(Buf *b, const char *s);
void buf_addf(Buf *b, const char *fmt, ...);
void buf_json_str(Buf *b, const char *s);   /* adds "..." with JSON escaping */

/* Read a whole file into a malloc'd, NUL-terminated block. NULL if missing/too big. */
char *file_read_all(const char *path, size_t *n, size_t max_bytes);

#endif