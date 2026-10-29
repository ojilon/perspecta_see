#ifndef API_H
#define API_H
#include "buf.h"
#include "server.h"

/* api.h — the few "words" the browser can say to C.
 *
 *   GET  /api/ping             -> {ok, cwd, root}      where are we?
 *   GET  /api/scan?root=PATH   -> the whole graph      (json.h)
 *   GET  /api/file?path=REL    -> raw text of a file   (text/plain)
 *   POST /api/save?path=REL    -> writes the body      {ok, bytes}
 *
 * Every path is relative to the scanned root and must pass path_allowed()
 * (fs.h), so the browser can only touch source files inside that folder.
 */
int api_handle(const Request *rq, Buf *out, const char **ctype);

#endif
