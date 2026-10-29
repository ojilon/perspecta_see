#ifndef PARSE_CSS_H
#define PARSE_CSS_H

/* parse_css.h — record the selectors a stylesheet defines.
 *
 * `.card`, `#tree` at the start of a rule become S_CLASS / S_ID symbols, so the
 * html and JS that mention the same name can be linked to this file.
 * `@import "x.css"` becomes an R_IMPORT reference.
 *
 * Input must already be mask_code()'d (comments blanked, strings kept only as
 * quotes) — see mask.h.
 */
void scan_css(int f, const char *masked, unsigned long n);

#endif
