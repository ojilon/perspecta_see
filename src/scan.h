#ifndef SCAN_H
#define SCAN_H
#include "buf.h"
#include <stddef.h>

/* Walk `root`, parse every html/css/js/c/h file, resolve links. Returns 1 on success. */
int scan_project(const char *root, char *err, size_t errsz);

/* The result of the last scan as JSON (what the browser receives). */
void scan_to_json(Buf *out);

const char *scan_root(void);          /* "" until the first scan */
const char *scan_cwd(void);           /* folder the server was started in */

/* Safety checks for paths that come from the browser (relative to the root). */
int path_is_safe(const char *rel);
int path_allowed(const char *rel);    /* only html/css/js/c/h files may be read or written */

#endif