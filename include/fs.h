#ifndef FS_H
#define FS_H
#include <stddef.h>

/* fs.h — everything that touches the disk or decides "is this path allowed?".
 *
 * Keeping it in one place means the safety rules are easy to audit: the server
 * may read and write only html/css/js/c/h files *inside* the scanned folder,
 * and every path coming from the browser is relative (no "..", no drive
 * letters, no backslashes).
 */

/* Language of a file name ("a/b/app.js" -> L_JS). L_OTHER for anything else. */
int lang_of(const char *path);

/* 1 if `rel` is a harmless relative path. 0 for absolute paths, "..", etc. */
int path_is_safe(const char *rel);

/* path_is_safe + the extension must be one we understand (editable file). */
int path_allowed(const char *rel);

/* Read a whole file into a malloc'd, NUL-terminated block. NULL if missing/too big. */
char *file_read_all(const char *path, size_t *n, size_t max_bytes);

/* Write `n` bytes to `path`. Returns the number written, or -1. */
long file_write_all(const char *path, const char *data, size_t n);

#endif
