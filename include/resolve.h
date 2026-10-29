#ifndef RESOLVE_H
#define RESOLVE_H

/* resolve.h — turn mentions into links.
 *
 * Parsing gives us references ("this file mentions ./app.js", "this function
 * calls draw"). Resolving happens once, after every file has been read, and
 * produces the two graphs the browser draws:
 *
 *   g_fedges  file  -> file     (include / script / style / import / class /
 *                                dom / api / href / call)
 *   g_calls   symbol -> symbol  (which function calls which, plus the html ids
 *                                and css classes its body touches)
 *
 * A reference that finds nothing is not thrown away: it lands in g_unres, the
 * "dangling links" list. That list is the parser's own to-do note.
 */
void resolve_all(void);

/* Look up a project-relative file ("web/js/app.js"). -1 if unknown. */
int resolve_find_file(const char *rel);

#endif
