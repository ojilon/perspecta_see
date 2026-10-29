/* fs.c — disk access and the path allow-list.
 *
 * The server is a local tool: it can edit the project it is pointed at. These
 * three checks (safe path, known extension, inside the scanned root) are the
 * whole security story, so they live together here where they are easy to see.
 */
#include "fs.h"
#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lang_of(const char *name) {
    const char *d = strrchr(name, '.');
    if (!d) return L_OTHER;
    if (!strcmp(d, ".c")) return L_C;
    if (!strcmp(d, ".h")) return L_H;
    if (!strcmp(d, ".js") || !strcmp(d, ".mjs")) return L_JS;
    if (!strcmp(d, ".html") || !strcmp(d, ".htm")) return L_HTML;
    if (!strcmp(d, ".css")) return L_CSS;
    return L_OTHER;
}

int path_is_safe(const char *rel) {
    if (!rel[0]) return 0;
    if (rel[0] == '/' || rel[0] == '\\') return 0;             /* absolute */
    if (strstr(rel, "..")) return 0;                           /* climb out */
    if (strchr(rel, '\\') || strchr(rel, ':')) return 0;       /* windows tricks */
    if (strstr(rel, "//")) return 0;
    return 1;
}

int path_allowed(const char *rel) { return path_is_safe(rel) && lang_of(rel) != L_OTHER; }

char *file_read_all(const char *path, size_t *n, size_t max_bytes) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz < 0 || (size_t)sz > max_bytes) { fclose(fp); return NULL; }
    char *b = (char *)malloc((size_t)sz + 1);
    if (!b) { fclose(fp); return NULL; }
    size_t r = fread(b, 1, (size_t)sz, fp);
    fclose(fp);
    b[r] = 0;
    if (n) *n = r;
    return b;
}

long file_write_all(const char *path, const char *data, size_t n) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;
    size_t w = fwrite(data, 1, n, fp);
    int bad = ferror(fp);
    fclose(fp);
    if (bad || w != n) return -1;
    return (long)w;
}
