/* scan.c: find the files, run the parser on each, then connect the dots. */
#include "scan.h"
#include "model.h"
#include "parse.h"
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

/* The data lives here, declared in model.h. Static = not on the stack. */
File  g_files[MAX_FILES];    int g_nfiles;
Sym   g_syms[MAX_SYMS];      int g_nsyms;
Ref   g_refs[MAX_REFS];      int g_nrefs;
FEdge g_fedges[MAX_FEDGES];  int g_nfedges;
Call  g_calls[MAX_CALLS];    int g_ncalls;
Unres g_unres[MAX_UNRES];    int g_nunres;

static char g_root[512];
static char g_cwd[512];
static int  g_ms;

static const char *LANG[] = { "other", "c", "h", "js", "html", "css" };
static const char *KIND[] = { "", "include", "script", "style", "import", "class", "dom", "api", "call" };

const char *scan_root(void) { return g_root; }
const char *scan_cwd(void) {
    if (!g_cwd[0]) {
        if (!getcwd(g_cwd, sizeof g_cwd)) strcpy(g_cwd, ".");
        for (char *p = g_cwd; *p; p++) if (*p == '\\') *p = '/';
    }
    return g_cwd;
}

/* ---------- which files count ---------- */
static int lang_of(const char *name) {
    const char *d = strrchr(name, '.');
    if (!d) return L_OTHER;
    if (!strcmp(d, ".c")) return L_C;
    if (!strcmp(d, ".h")) return L_H;
    if (!strcmp(d, ".js") || !strcmp(d, ".mjs")) return L_JS;
    if (!strcmp(d, ".html") || !strcmp(d, ".htm")) return L_HTML;
    if (!strcmp(d, ".css")) return L_CSS;
    return L_OTHER;
}

static int ignored_dir(const char *n) {
    static const char *skip[] = { "node_modules", "build", "dist", "target", "out", "zig-out", "vendor", "third_party", NULL };
    for (int i = 0; skip[i]; i++) if (!strcmp(n, skip[i])) return 1;
    return 0;
}

int path_is_safe(const char *rel) {
    if (!rel[0] || rel[0] == '/' || strstr(rel, "..") || strchr(rel, '\\') || strchr(rel, ':') || strstr(rel, "//")) return 0;
    return 1;
}
int path_allowed(const char *rel) { return path_is_safe(rel) && lang_of(rel) != L_OTHER; }

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
        }
    }
    closedir(d);
}

static int cmp_file(const void *a, const void *b) { return strcmp(((const File *)a)->rel, ((const File *)b)->rel); }

/* ---------- resolving links between files ---------- */
static int find_file(const char *rel) {
    for (int i = 0; i < g_nfiles; i++) if (!strcmp(g_files[i].rel, rel)) return i;
    return -1;
}

/* a file whose path ends with `suf` ("js/app.js" matches "web/js/app.js") */
static int find_suffix(const char *suf) {
    size_t sl = strlen(suf);
    for (int i = 0; i < g_nfiles; i++) {
        size_t l = strlen(g_files[i].rel);
        if (l >= sl && !strcmp(g_files[i].rel + l - sl, suf) && (l == sl || g_files[i].rel[l - sl - 1] == '/')) return i;
    }
    return -1;
}

/* directory of `from_rel` + target, with "." and ".." folded away */
static void join_norm(const char *from_rel, const char *target, char *out, size_t outsz) {
    char tmp[512], *parts[64];
    int np = 0;
    const char *sl = strrchr(from_rel, '/');
    if (sl) snprintf(tmp, sizeof tmp, "%.*s/%s", (int)(sl - from_rel), from_rel, target);
    else snprintf(tmp, sizeof tmp, "%s", target);
    for (char *t = strtok(tmp, "/"); t; t = strtok(NULL, "/")) {
        if (!*t || !strcmp(t, ".")) continue;
        if (!strcmp(t, "..")) { if (np > 0) np--; continue; }
        if (np < 64) parts[np++] = t;
    }
    out[0] = 0;
    for (int i = 0; i < np; i++) {
        if (i) strncat(out, "/", outsz - strlen(out) - 1);
        strncat(out, parts[i], outsz - strlen(out) - 1);
    }
}

static int resolve_path(int f, const char *t, int kind) {
    char buf[520];
    int r;
    if (t[0] == '/') return find_suffix(t + 1);
    join_norm(g_files[f].rel, t, buf, sizeof buf - 4);
    if ((r = find_file(buf)) >= 0) return r;
    if (kind == R_IMPORT) {
        strcat(buf, ".js");
        if ((r = find_file(buf)) >= 0) return r;
    }
    if (kind == R_INCLUDE) {                               /* "waydirect/x.h": search include folders */
        while (!strncmp(t, "./", 2) || !strncmp(t, "../", 3)) t += (t[1] == '/') ? 2 : 3;
        return find_suffix(t);
    }
    return -1;
}

static void add_fedge(int from, int to, int kind, const char *label) {
    if (from == to) return;
    for (int i = 0; i < g_nfedges; i++)
        if (g_fedges[i].from == from && g_fedges[i].to == to && g_fedges[i].kind == kind) { g_fedges[i].n++; return; }
    if (g_nfedges >= MAX_FEDGES) return;
    FEdge *e = &g_fedges[g_nfedges++];
    e->from = from; e->to = to; e->kind = kind; e->n = 1;
    snprintf(e->label, sizeof e->label, "%s", label);
}

static void add_call(int from, int to, int kind) {
    if (from < 0 || to < 0 || from == to) return;
    for (int i = 0; i < g_ncalls; i++)
        if (g_calls[i].from == from && g_calls[i].to == to && g_calls[i].kind == kind) return;
    if (g_ncalls >= MAX_CALLS) return;
    g_calls[g_ncalls++] = (Call){ from, to, kind };
}

static int family(int lang) { return (lang == L_C || lang == L_H) ? 1 : (lang == L_JS ? 2 : 0); }

static int find_sym(int kind, const char *name, int from_file, int lang_family, int want_lang) {
    int best = -1;
    for (int i = 0; i < g_nsyms; i++) {
        Sym *s = &g_syms[i];
        if (s->kind != kind || s->name[0] != name[0] || strcmp(s->name, name)) continue;
        int lg = g_files[s->file].lang;
        if (lang_family && family(lg) != lang_family) continue;
        if (want_lang && lg != want_lang) continue;
        if (s->file == from_file) return i;
        if (best < 0) best = i;
    }
    return best;
}

static void unresolved(const Ref *r) {
    if (g_nunres >= MAX_UNRES) return;
    Unres *u = &g_unres[g_nunres++];
    u->file = r->file; u->kind = r->kind; u->line = r->line;
    snprintf(u->target, sizeof u->target, "%s", r->target);
}

static void resolve_all(void) {
    for (int i = 0; i < g_nrefs; i++) {
        Ref *r = &g_refs[i];
        int to = -1, sym = -1;
        switch (r->kind) {
        case R_INCLUDE: case R_SCRIPT: case R_STYLE: case R_IMPORT:
            to = resolve_path(r->file, r->target, r->kind);
            if (to >= 0) add_fedge(r->file, to, r->kind, r->target); else unresolved(r);
            break;
        case R_CLASS:
            sym = find_sym(S_CLASS, r->target, r->file, 0, L_CSS);
            break;
        case R_DOM:
            sym = find_sym(S_ID, r->target, r->file, 0, L_HTML);
            break;
        case R_API:
            sym = find_sym(S_ROUTE, r->target, r->file, 0, 0);
            if (sym < 0) unresolved(r);
            break;
        case R_CALL:
            sym = find_sym(S_FUNC, r->target, r->file, family(g_files[r->file].lang), 0);
            break;
        }
        if (sym >= 0) {
            add_fedge(r->file, g_syms[sym].file, r->kind, r->target);
            add_call(r->from_sym, sym, r->kind);
        }
    }
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
    g_nfiles = g_nsyms = g_nrefs = g_nfedges = g_ncalls = g_nunres = 0;

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

/* ---------- JSON for the browser ---------- */
void scan_to_json(Buf *o) {
    buf_add(o, "{\"root\":");
    buf_json_str(o, g_root);

    buf_add(o, ",\"files\":[");
    for (int i = 0; i < g_nfiles; i++) {
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"id\":%d,\"lang\":\"%s\",\"lines\":%d,\"path\":", i, LANG[g_files[i].lang], g_files[i].lines);
        buf_json_str(o, g_files[i].rel);
        buf_add(o, "}");
    }
    buf_add(o, "],\"syms\":[");
    static const char *SK[] = { "", "func", "route", "id", "class" };
    for (int i = 0; i < g_nsyms; i++) {
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"id\":%d,\"file\":%d,\"kind\":\"%s\",\"line\":%d,\"name\":", i, g_syms[i].file, SK[g_syms[i].kind], g_syms[i].line);
        buf_json_str(o, g_syms[i].name);
        buf_add(o, "}");
    }
    buf_add(o, "],\"edges\":[");
    for (int i = 0; i < g_nfedges; i++) {
        FEdge *e = &g_fedges[i];
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"from\":%d,\"to\":%d,\"kind\":\"%s\",\"n\":%d,\"label\":", e->from, e->to, KIND[e->kind], e->n);
        buf_json_str(o, e->label);
        buf_add(o, "}");
    }
    buf_add(o, "],\"calls\":[");
    for (int i = 0; i < g_ncalls; i++) {
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"from\":%d,\"to\":%d,\"kind\":\"%s\"}", g_calls[i].from, g_calls[i].to, KIND[g_calls[i].kind]);
    }
    buf_add(o, "],\"unresolved\":[");
    for (int i = 0; i < g_nunres; i++) {
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"file\":%d,\"line\":%d,\"kind\":\"%s\",\"target\":", g_unres[i].file, g_unres[i].line, KIND[g_unres[i].kind]);
        buf_json_str(o, g_unres[i].target);
        buf_add(o, "}");
    }
    buf_addf(o, "],\"stats\":{\"files\":%d,\"syms\":%d,\"edges\":%d,\"ms\":%d}}", g_nfiles, g_nsyms, g_nfedges, g_ms);
}