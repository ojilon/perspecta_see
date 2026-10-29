/* store.c — the static "database" (declared in model.h) plus the small
 * functions every parser uses to put something into it.
 *
 * Static arrays, no allocation: a second scan simply resets the counters.
 */
#include "store.h"
#include "model.h"
#include <string.h>

File  g_files[MAX_FILES];    int g_nfiles;
Sym   g_syms[MAX_SYMS];      int g_nsyms;
Ref   g_refs[MAX_REFS];      int g_nrefs;
FEdge g_fedges[MAX_FEDGES];  int g_nfedges;
Call  g_calls[MAX_CALLS];    int g_ncalls;
Unres g_unres[MAX_UNRES];    int g_nunres;

static int g_file_first_sym;      /* first symbol index that belongs to the current file */

/* Which symbols can a mention of this language reach?
 *
 *   C / H  -> family 1 (both, they are the same world)
 *   JS     -> family 2
 *   HTML   -> family 2 as well: the only functions an html file defines come
 *             from its inline <script> blocks, and those are JavaScript, so a
 *             call in a .js file must be allowed to find them.
 *   CSS    -> 0 (classes and ids are looked up by language, not by family)
 */
int lang_family(int lang) {
    if (lang == L_C || lang == L_H) return 1;
    if (lang == L_JS || lang == L_HTML) return 2;
    return 0;
}

void store_reset(void) {
    g_nfiles = g_nsyms = g_nrefs = g_nfedges = g_ncalls = g_nunres = 0;
    g_file_first_sym = 0;
}

void store_begin_file(void) { g_file_first_sym = g_nsyms; }
int  store_ref_mark(void)   { return g_nrefs; }

int store_add_sym(int file, int kind, const char *name, size_t nl, int line) {
    if (nl > 63) nl = 63;
    if (kind != S_FUNC)                         /* ids, classes, routes: one per name per file */
        for (int i = g_file_first_sym; i < g_nsyms; i++)
            if (g_syms[i].kind == kind && strlen(g_syms[i].name) == nl &&
                !strncmp(g_syms[i].name, name, nl))
                return i;
    if (g_nsyms >= MAX_SYMS) return -1;
    Sym *s = &g_syms[g_nsyms];
    s->file = file; s->kind = kind; s->line = line; s->end = line;
    memcpy(s->name, name, nl);
    s->name[nl] = 0;
    return g_nsyms++;
}

void store_set_end(int sym, int end) {
    if (sym >= 0 && sym < g_nsyms && end > g_syms[sym].end) g_syms[sym].end = end;
}

void store_add_ref(int file, int from_sym, int kind, const char *t, size_t tl, int line) {
    if (g_nrefs >= MAX_REFS) return;
    if (tl > 127) tl = 127;
    Ref *r = &g_refs[g_nrefs++];
    r->file = file; r->from_sym = from_sym; r->kind = kind; r->line = line;
    memcpy(r->target, t, tl);
    r->target[tl] = 0;
}

int store_enclosing(int line) {
    for (int i = g_file_first_sym; i < g_nsyms; i++)
        if (g_syms[i].kind == S_FUNC && line >= g_syms[i].line && line <= g_syms[i].end) return i;
    return -1;
}

int store_sym_at_line(int file, int line) {
    for (int i = 0; i < g_nsyms; i++)
        if (g_syms[i].file == file && g_syms[i].line == line) return i;
    return -1;
}
