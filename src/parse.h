#ifndef PARSE_H
#define PARSE_H
#include <stddef.h>

/* Read one file's text (src, NUL-terminated, n bytes) and record what it
   defines (g_syms) and what it mentions (g_refs). File g_files[f] must already exist. */
void parse_file(int f, const char *src, size_t n);

#endif