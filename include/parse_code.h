#ifndef PARSE_CODE_H
#define PARSE_CODE_H
#include <stddef.h>

/* parse_code.h — the C / JavaScript walker.
 *
 * Input must already be mask_code()'d (comments and string bodies blanked),
 * so all that is left are identifiers, punctuation and whitespace. From that
 * we pick up:
 *
 *   definitions   `int main(...) {`        (C, at top level)
 *                 `function draw() {`      (JS)
 *                 `const draw = () => {`   (JS, at top level)
 *                 `draw() {`               (JS class / object literal method)
 *   scopes        the line range of every body, closed when its `}` is seen
 *   calls         `name(` inside a body, including `obj.name(` (R_MCALL)
 *
 * Not a grammar: macros that generate functions are invisible, and a call
 * whose name is not defined anywhere is dropped silently.
 */
void scan_code(int f, int lang, const char *masked, size_t n);

#endif
