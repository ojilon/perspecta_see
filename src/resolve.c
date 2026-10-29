/* resolve.c — connect the dots.
 *
 * Two lookups, in this order of preference:
 *   1. inside the same file        (a call usually goes to the sibling above it)
 *   2. anywhere in the same family (C/H together, JS on its own)
 *
 * A member call (obj.name(...)) only ever resolves inside its own file. That
 * keeps `list.get(x)` from being drawn as a link to an unrelated `get()` in
 * some other file — weak evidence, weak link.
 */
#include "resolve.h"
#include "model.h"
#include "store.h"
#include <stdio.h>
#include <string.h>

static int find_file(const char *rel) {
    for (int i = 0; i < g_nfiles; i++) if (!strcmp(g_files[i].rel, rel)) return i;
    return -1;
}

int resolve_find_file(const char *rel) { return find_file(rel); }

/* a file whose path ends with `suf` ("js/app.js" matches "web/js/app.js") */
static int find_suffix(const char *suf) {
    size_t sl = strlen(suf);
    for (int i = 0; i < g_nfiles; i++) {
        size_t l = strlen(g_files[i].rel);
        if (l >= sl && !strcmp(g_files[i].rel + l - sl, suf) &&
            (l == sl || g_files[i].rel[l - sl - 1] == '/')) return i;
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
    if (t[0] == '/') return find_suffix(t + 1);      /* "/js/app.js": from the root */
    join_norm(g_files[f].rel, t, buf, sizeof buf - 4);
    if ((r = find_file(buf)) >= 0) return r;
    if (kind == R_IMPORT) {
        strcat(buf, ".js");                          /* import "./x" without extension */
        if ((r = find_file(buf)) >= 0) return r;
    }
    if (kind == R_INCLUDE) {                         /* "waydirect/x.h": search include folders */
        while (!strncmp(t, "./", 2) || !strncmp(t, "../", 3)) t += (t[1] == '/') ? 2 : 3;
        return find_suffix(t);
    }
    return -1;
}

static void add_fedge(int from, int to, int kind, const char *label) {
    if (from == to) return;                          /* same file: nothing to draw */
    for (int i = 0; i < g_nfedges; i++)
        if (g_fedges[i].from == from && g_fedges[i].to == to && g_fedges[i].kind == kind) {
            g_fedges[i].n++;
            return;
        }
    if (g_nfedges >= MAX_FEDGES) return;
    FEdge *e = &g_fedges[g_nfedges++];
    e->from = from; e->to = to; e->kind = kind; e->n = 1;
    snprintf(e->label, sizeof e->label, "%s", label);
}

static void add_call(int from, int to, int kind) {
    if (from < 0 || to < 0) return;
    for (int i = 0; i < g_ncalls; i++)
        if (g_calls[i].from == from && g_calls[i].to == to && g_calls[i].kind == kind) return;
    if (g_ncalls >= MAX_CALLS) return;
    g_calls[g_ncalls].from = from;
    g_calls[g_ncalls].to = to;
    g_calls[g_ncalls].kind = kind;
    g_ncalls++;
}

static int find_sym(int kind, const char *name, int from_file, int want_family,
                    int want_lang, int same_file_only) {
    int best = -1;
    for (int i = 0; i < g_nsyms; i++) {
        Sym *s = &g_syms[i];
        if (s->kind != kind || s->name[0] != name[0] || strcmp(s->name, name)) continue;
        int lg = g_files[s->file].lang;
        if (want_family && lang_family(lg) != want_family) continue;
        if (want_lang && lg != want_lang) continue;
        if (s->file == from_file) return i;
        if (same_file_only) continue;
        if (best < 0) best = i;
    }
    return best;
}

static void unresolved(const Ref *r) {
    if (g_nunres >= MAX_UNRES) return;
    Unres *u = &g_unres[g_nunres++];
    u->file = r->file;
    u->kind = r->kind;
    u->line = r->line;
    snprintf(u->target, sizeof u->target, "%s", r->target);
}

void resolve_all(void) {
    for (int i = 0; i < g_nrefs; i++) {
        Ref *r = &g_refs[i];
        int to = -1, sym = -1;

        switch (r->kind) {
        case R_INCLUDE: case R_SCRIPT: case R_STYLE: case R_IMPORT: case R_HREF:
            to = resolve_path(r->file, r->target, r->kind);
            if (to >= 0) add_fedge(r->file, to, r->kind, r->target);
            else unresolved(r);
            break;
        case R_CLASS:
            sym = find_sym(S_CLASS, r->target, r->file, 0, L_CSS, 0);
            break;
        case R_DOM:
            sym = find_sym(S_ID, r->target, r->file, 0, L_HTML, 0);
            break;
        case R_API:
            sym = find_sym(S_ROUTE, r->target, r->file, 0, 0, 0);
            if (sym < 0) unresolved(r);              /* a fetch() with no route is a real bug */
            break;
        case R_CALL:
            sym = find_sym(S_FUNC, r->target, r->file, lang_family(g_files[r->file].lang), 0, 0);
            break;
        case R_MCALL:
            sym = find_sym(S_FUNC, r->target, r->file, lang_family(g_files[r->file].lang), 0, 1);
            break;
        }
        if (sym >= 0) {
            add_fedge(r->file, g_syms[sym].file, r->kind, r->target);
            add_call(r->from_sym, sym, r->kind);     /* from == to means recursion */
        }
    }
}
