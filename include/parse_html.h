#ifndef PARSE_HTML_H
#define PARSE_HTML_H
#include <stddef.h>

/* parse_html.h — the HTML scanner.
 *
 * Records:
 *   id="..."                 -> S_ID symbol
 *   class="a b c"            -> three R_CLASS references
 *   <script src="x.js">      -> R_SCRIPT reference
 *   <link href="x.css">      -> R_STYLE reference
 *   <a href="page.html">     -> R_HREF reference
 *   <script> ... </script>   -> the body is handed to the JS parser (inline code!)
 *   <style> ... </style>     -> the body is handed to the CSS parser
 *
 * The inline blocks are why a page that keeps its code in one file still gets
 * function-level links: the body is parsed as if it were a .js file, but the
 * line numbers still point at the html file.
 */
void parse_html(int f, const char *src, size_t n);

#endif
