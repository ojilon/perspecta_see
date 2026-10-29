#ifndef SERVER_H
#define SERVER_H
#include "buf.h"
#include <stddef.h>

/* server.h — a very small HTTP/1.1 server.
 *
 * One request at a time, one connection per request, no keep-alive: the point
 * is to be readable, not fast. GET serves web/ from disk, /api/... is handed to
 * the ApiHandler in api.h.
 */

typedef struct {
    char method[8];
    char path[512];      /* "/api/scan" (already URL-decoded) */
    char query[1024];    /* "root=D%3A%2Fprojects" (still encoded) */
    char *body;          /* POST body: use body_len, it is not NUL-terminated */
    size_t body_len;
} Request;

/* Fill `out`, set *ctype if it is not JSON, return the HTTP status code. */
typedef int (*ApiHandler)(const Request *rq, Buf *out, const char **ctype);

/* Blocks forever. `host` is "127.0.0.1" for normal use; "0.0.0.0" makes the
   server reachable from other machines (only useful for a demo sandbox, and
   it means anyone on the network can edit the files you scanned — so it stays
   an explicit flag). */
int server_run(int port, const char *host, const char *web_dir, ApiHandler api);

/* Read ?key=value from a query string (decoded). Returns 1 if found. */
int query_get(const char *query, const char *key, char *out, size_t outsz);

#endif
