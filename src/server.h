#ifndef SERVER_H
#define SERVER_H
#include "buf.h"
#include <stddef.h>

typedef struct {
    char method[8];
    char path[512];      /* "/api/scan" */
    char query[1024];    /* "root=D%3A%2Fprojects" (still encoded) */
    char *body;          /* POST body (not NUL-terminated safe: use body_len) */
    size_t body_len;
} Request;

/* Fill `out`, set *ctype if not JSON, return the HTTP status code. */
typedef int (*ApiHandler)(const Request *rq, Buf *out, const char **ctype);

/* Blocks forever: serves files from web_dir and calls `api` for /api/... */
int server_run(int port, const char *web_dir, ApiHandler api);

/* Read ?key=value from a query string (decoded). Returns 1 if found. */
int query_get(const char *query, const char *key, char *out, size_t outsz);

#endif