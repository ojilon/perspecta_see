#ifndef MASK_H
#define MASK_H
#include <stddef.h>

/* mask.h — "hide the parts of the text that are not code".
 *
 * All three functions rewrite a string *in place*, keep its length and keep
 * every newline, so byte offsets and line numbers stay valid afterwards. That
 * is the whole trick behind the parser: the scans can then use plain strstr()
 * without worrying about what is code and what is prose.
 *
 *   mask_comments — blank comments only, keep string literals.
 *                   Used by the raw-text scans (include paths, /api routes,
 *                   getElementById("..."), $("...")). Reading comments used to
 *                   invent routes and DOM ids that do not exist; this fixes it.
 *
 *   mask_code     — blank comments AND the inside of strings. Used by the code
 *                   walker, which only wants identifiers: `if (x == "{")` must
 *                   not look like a block that opens.
 *
 *   mask_preproc  — blank C preprocessor lines (#define, #include, ...) so a
 *                   macro body is not mistaken for a function definition.
 */

void mask_comments(char *s, size_t n, int lang);
void mask_code(char *s, size_t n, int lang);
void mask_preproc(char *s, size_t n);

#endif
