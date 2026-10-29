/* api.c — the bridge between the browser and the scanner.
 *
 * Four endpoints, each one a thin wrapper around a function that already
 * exists elsewhere: scan_project (scan.h), scan_to_json (json.h),
 * file_read_all / file_write_all (fs.h).
 *
 * Anything that comes from the browser is a relative path validated by
 * path_allowed(), so the worst a request can do is rewrite a source file
 * inside the folder the user scanned.
 */
#include "api.h"
#include "fs.h"
#include "json.h"
#include "model.h"
#include "scan.h"
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

int api_handle(const Request *rq, Buf *out, const char **ctype) {
    char a[1024], err[300], full[1600];

    /* GET /api/ping -> where are we, what is scanned right now */
    if (!strcmp(rq->path, "/api/ping")) {
        buf_add(out, "{\"ok\":true,\"cwd\":");
        buf_json_str(out, scan_cwd());
        buf_add(out, ",\"root\":");
        buf_json_str(out, scan_root());
        buf_add(out, "}");
        return 200;
    }

    /* GET /api/scan?root=PATH -> parse the project, return files/symbols/links */
    if (!strcmp(rq->path, "/api/scan")) {
        if (!query_get(rq->query, "root", a, sizeof a)) strcpy(a, ".");
        if (!scan_project(a, err, sizeof err)) return fail(out, ctype, 400, err);
        scan_to_json(out);
        return 200;
    }

    /* GET /api/file?path=REL -> raw text of one file */
    if (!strcmp(rq->path, "/api/file")) {
        if (!query_get(rq->query, "path", a, sizeof a) || !path_allowed(a))
            return fail(out, ctype, 400, "bad path");
        if (!scan_root()[0]) return fail(out, ctype, 400, "scan a project first");
        full_path(a, full, sizeof full);
        size_t n = 0;
        char *data = file_read_all(full, &n, MAX_FILE_BYTES);
        if (!data) return fail(out, ctype, 404, "cannot read file");
        *ctype = "text/plain; charset=utf-8";
        buf_addn(out, data, n);
        free(data);
        return 200;
    }

    /* POST /api/save?path=REL  (body = new file text) -> write it to disk */
    if (!strcmp(rq->path, "/api/save")) {
        if (strcmp(rq->method, "POST")) return fail(out, ctype, 405, "POST only");
        if (!scan_root()[0]) return fail(out, ctype, 400, "scan a project first");
        if (!query_get(rq->query, "path", a, sizeof a) || !path_allowed(a))
            return fail(out, ctype, 400, "bad path");
        full_path(a, full, sizeof full);
        long w = file_write_all(full, rq->body, rq->body_len);
        if (w < 0) return fail(out, ctype, 500, "cannot write file");
        buf_addf(out, "{\"ok\":true,\"bytes\":%ld}", w);
        return 200;
    }

    return fail(out, ctype, 404, "unknown api");
}
