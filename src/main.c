/* main.c: start-up + the API: the few "words" the browser can say to C. */
#include "buf.h"
#include "scan.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fail(Buf *out, const char **ctype, int status, const char *msg) {
    *ctype = "text/plain; charset=utf-8";
    buf_add(out, msg);
    return status;
}

/* Full path on disk for a project-relative path. */
static void full_path(const char *rel, char *out, size_t outsz) {
    snprintf(out, outsz, "%s/%s", scan_root(), rel);
}

static int api(const Request *rq, Buf *out, const char **ctype) {
    char a[1024], err[300], full[1600];

    /* GET /api/ping  ->  where are we, what is scanned right now */
    if (!strcmp(rq->path, "/api/ping")) {
        buf_add(out, "{\"ok\":true,\"cwd\":");
        buf_json_str(out, scan_cwd());
        buf_add(out, ",\"root\":");
        buf_json_str(out, scan_root());
        buf_add(out, "}");
        return 200;
    }

    /* GET /api/scan?root=PATH  ->  parse the project, return files/symbols/links */
    if (!strcmp(rq->path, "/api/scan")) {
        if (!query_get(rq->query, "root", a, sizeof a)) strcpy(a, ".");
        if (!scan_project(a, err, sizeof err)) return fail(out, ctype, 400, err);
        scan_to_json(out);
        return 200;
    }

    /* GET /api/file?path=REL  ->  raw text of one file */
    if (!strcmp(rq->path, "/api/file")) {
        if (!query_get(rq->query, "path", a, sizeof a) || !path_allowed(a)) return fail(out, ctype, 400, "bad path");
        full_path(a, full, sizeof full);
        size_t n = 0;
        char *data = file_read_all(full, &n, 1024 * 1024);
        if (!data) return fail(out, ctype, 404, "cannot read file");
        *ctype = "text/plain; charset=utf-8";
        buf_addn(out, data, n);
        free(data);
        return 200;
    }

    /* POST /api/save?path=REL  (body = new file text)  ->  write it to disk */
    if (!strcmp(rq->path, "/api/save")) {
        if (strcmp(rq->method, "POST")) return fail(out, ctype, 405, "POST only");
        if (!scan_root()[0]) return fail(out, ctype, 400, "scan a project first");
        if (!query_get(rq->query, "path", a, sizeof a) || !path_allowed(a)) return fail(out, ctype, 400, "bad path");
        full_path(a, full, sizeof full);
        FILE *fp = fopen(full, "wb");
        if (!fp) return fail(out, ctype, 500, "cannot open file for writing");
        size_t w = fwrite(rq->body, 1, rq->body_len, fp);
        fclose(fp);
        if (w != rq->body_len) return fail(out, ctype, 500, "write failed");
        buf_addf(out, "{\"ok\":true,\"bytes\":%lu}", (unsigned long)w);
        return 200;
    }

    return fail(out, ctype, 404, "unknown api");
}

int main(int argc, char **argv) {
    int port = 8787;
    const char *web = "web";
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--port") && i + 1 < argc) port = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--web") && i + 1 < argc) web = argv[++i];
        else { fprintf(stderr, "usage: perspecta_see [--port N] [--web DIR]\n"); return 1; }
    }
    return server_run(port, web, api);
}