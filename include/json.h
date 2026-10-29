#ifndef JSON_H
#define JSON_H
#include "buf.h"

/* json.h — the wire format.
 *
 * One object with seven keys; the browser builds everything it shows from it.
 *
 *   { "root": "/home/me/project",
 *     "files":      [ {id, path, lang, lines, bytes} ],
 *     "syms":       [ {id, file, kind, name, line, end} ],       line..end = scope
 *     "edges":      [ {from, to, kind, n, label} ],              file -> file
 *     "calls":      [ {from, to, kind} ],                        symbol -> symbol
 *     "unresolved": [ {file, line, kind, target} ],              dangling links
 *     "stats":      {files, syms, refs, edges, calls, unresolved, ms} }
 *
 * Kept flat and tiny on purpose: no nesting deeper than one level, so the
 * JavaScript side can stay a few lines per feature.
 */
void scan_to_json(Buf *out);

#endif
