/* scan.c — find the files, run the parser on each, then connect the dots.
 *
 * One scan = walk + parse + resolve, in that order, because a reference can
 * only be resolved once every file has been seen. The timing reported to the
 * browser is real CPU time, which makes it easy to notice a directory that
 * should have been skipped.
 */
#include "scan.h"
#include "fs.h"
#include "model.h"
#include "parse.h"
#include "resolve.h"
#include "store.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

static char g_root[512];
static char g_cwd[512];
static int  g_ms;

const char *scan_root(void) { return g_root; }
int scan_ms(void) { return g_ms; }

const char *scan_cwd(void) {
    if (!g_cwd[0]) {
        if (!getcwd(g_cwd, sizeof g_cwd)) strcpy(g_cwd, ".");
        for (char *p = g_cwd; *p; p++) if (*p == '\\') *p = '/';
    }
    return g_cwd;
}

/* ---------- which folders to skip ---------- */
static int ignored_dir(const char *n) {
    static const char *skip[] = { "node_modules", "build", "dist", "target", "out",
                                  "zig-out", "vendor", "third_party", "obj", ".git", NULL };
    for (int i = 0; skip[i]; i++) if (!strcmp(n, skip[i])) return 1;
    return 0;
}

/* ---------- walking the folder ---------- */
static void walk(const char *rel, int depth) {
    if (depth > 12) return;
    char abs[1100];
    snprintf(abs, sizeof abs, "%.600s%s%.300s", g_root, rel[0] ? "/" : "", rel);
    DIR *d = opendir(abs);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        const char *nm = e->d_name;
        if (nm[0] == '.') continue;                       /* ., .., .git, hidden stuff */
        char crel[256], cabs[1100];
        if ((size_t)snprintf(crel, sizeof crel, "%s%s%s", rel, rel[0] ? "/" : "", nm) >= sizeof crel) continue;
        snprintf(cabs, sizeof cabs, "%.600s/%s", g_root, crel);
        struct stat st;
        if (stat(cabs, &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            if (!ignored_dir(nm)) walk(crel, depth + 1);
        } else if (S_ISREG(st.st_mode)) {
            int lg = lang_of(nm);
            if (lg == L_OTHER || st.st_size > MAX_FILE_BYTES || g_nfiles >= MAX_FILES) continue;
            File *f = &g_files[g_nfiles++];
            snprintf(f->rel, sizeof f->rel, "%s", crel);
            f->lang = lg;
            f->bytes = (long)st.st_size;
            f->lines = 0;
        }
    }
    closedir(d);
}

static int cmp_file(const void *a, const void *b) {
    return strcmp(((const File *)a)->rel, ((const File *)b)->rel);
}

/* ---------- the whole scan ---------- */
int scan_project(const char *root, char *err, size_t errsz) {
    char r[512];
    if (!root[0] || !strcmp(root, ".")) snprintf(r, sizeof r, "%s", scan_cwd());
    else snprintf(r, sizeof r, "%s", root);
    for (char *p = r; *p; p++) if (*p == '\\') *p = '/';
    size_t l = strlen(r);
    while (l > 1 && r[l - 1] == '/') r[--l] = 0;

    struct stat st;
    if (stat(r, &st) != 0 || !S_ISDIR(st.st_mode)) {
        snprintf(err, errsz, "Not a folder: %s", r);
        return 0;
    }

    clock_t t0 = clock();
    snprintf(g_root, sizeof g_root, "%s", r);
    store_reset();

    walk("", 0);
    qsort(g_files, (size_t)g_nfiles, sizeof(File), cmp_file);

    for (int i = 0; i < g_nfiles; i++) {
        char path[1100];
        snprintf(path, sizeof path, "%.600s/%.255s", g_root, g_files[i].rel);
        size_t n = 0;
        char *src = file_read_all(path, &n, MAX_FILE_BYTES);
        if (!src) continue;
        int lines = 1;
        for (size_t k = 0; k < n; k++) if (src[k] == '\n') lines++;
        g_files[i].lines = lines;
        parse_file(i, src, n);
        free(src);
    }
    resolve_all();
    g_ms = (int)((clock() - t0) * 1000 / CLOCKS_PER_SEC);
    return 1;
}
