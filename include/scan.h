#ifndef SCAN_H
#define SCAN_H
#include <stddef.h>

/* scan.h — the whole job: walk a folder, parse every file, resolve the links.
 *
 * This is the only entry point the HTTP layer uses. It is synchronous and
 * single-shot: the result stays in the global arrays until the next scan.
 */

/* Walk `root`, parse every html/css/js/c/h file, resolve links.
   Returns 1 on success; on failure writes a message into `err`. */
int scan_project(const char *root, char *err, size_t errsz);

const char *scan_root(void);          /* "" until the first scan */
const char *scan_cwd(void);           /* folder the server was started in */
int scan_ms(void);                    /* how long the last scan took */

#endif
