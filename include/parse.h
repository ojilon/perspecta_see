#ifndef PARSE_H
#define PARSE_H
#include <stddef.h>

/* parse.h — the front door of the parser.
 *
 * parse_file() is what src/scan.c calls for every file it finds. It picks the
 * pipeline for the language, runs the raw-text scans on a comment-masked copy,
 * runs the code walker on a fully masked copy, and finally ties every loose
 * reference to the function it sits in.
 *
 * The two text entry points exist because an HTML file can contain code:
 * parse_html() calls them for inline <script> and <style> bodies, passing a
 * copy of the page in which only that body is visible. Line numbers therefore
 * still point into the html file.
 */
void parse_file(int f, const char *src, size_t n);

void parse_js_text(int f, const char *src, size_t n);   /* .js, or inline <script> */
void parse_css_text(int f, const char *src, size_t n);  /* .css, or inline <style> */

#endif
