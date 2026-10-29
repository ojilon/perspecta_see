#ifndef STORE_H
#define STORE_H
#include <stddef.h>

/* store.h — the write side of model.h.
 *
 * Parsers never touch the arrays directly: they call store_add_sym() and
 * store_add_ref(), which take care of the ceilings, of truncating long names
 * and of the "one id/class per file" de-duplication. Two marks
 * (store_sym_mark / store_ref_mark) delimit "what this file added", which is
 * what the enclosing-function pass needs.
 */

void store_reset(void);            /* start a new scan: all counters to 0 */

void store_begin_file(void);       /* new file: remember where its symbols start */
int  store_ref_mark(void);         /* index of the next ref to be added */

/* Returns the index of the new (or existing, for ids/classes) symbol, or -1. */
int  store_add_sym(int file, int kind, const char *name, size_t nl, int line);
void store_set_end(int sym, int end);   /* close the scope of a function */

void store_add_ref(int file, int from_sym, int kind, const char *t, size_t tl, int line);

/* Which function of the file being parsed contains `line`? -1 if none. */
int  store_enclosing(int line);

/* Index of the symbol that starts at `line` in `file` (for scope links). -1 if none. */
int  store_sym_at_line(int file, int line);

#endif
