#ifndef PATTERNS_H
#define PATTERNS_H
#include <stddef.h>

/* patterns.h — raw-text scans.
 *
 * These look for a *shape* in the text and read the string literal that
 * follows it: `getElementById("tree")`, `$("#tree")`, `<script src="app.js">`.
 * They are cheap and they catch the links a code walker never sees (a route is
 * just a string in C, an id is just a string in JS).
 *
 * IMPORTANT: call them with a text that mask_comments() has already blanked,
 * otherwise a comment that mentions "/api/scan" becomes a fake route.
 */

/* #include "local.h"  (angle-bracket system headers are ignored) */
void scan_includes(int f, const char *masked);

/* "/api/..." inside a string. In a C file it is a route definition, in a JS
   file it is a call site — that is the frontend/backend bridge. */
void scan_routes(int f, const char *masked, int as_definition);

/* JS only: getElementById / $() / querySelector / classList / import specifiers. */
void scan_dom_refs(int f, const char *masked);

#endif
