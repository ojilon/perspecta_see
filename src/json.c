/* json.c — serialise the last scan for the browser.
 *
 * The shapes and their short names are the contract between C and JS; if you
 * add a field here, add it to include/json.h and read it in web/js/state.js.
 */
#include "json.h"
#include "model.h"
#include "scan.h"

static const char *LANG[] = { "other", "c", "h", "js", "html", "css" };
static const char *SYMK[] = { "", "func", "route", "id", "class" };
static const char *KIND[] = { "", "include", "script", "style", "import",
                              "class", "dom", "api", "call", "href", "mcall" };

void scan_to_json(Buf *o) {
    buf_add(o, "{\"root\":");
    buf_json_str(o, scan_root());

    buf_add(o, ",\"files\":[");
    for (int i = 0; i < g_nfiles; i++) {
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"id\":%d,\"lang\":\"%s\",\"lines\":%d,\"bytes\":%ld,\"path\":",
                 i, LANG[g_files[i].lang], g_files[i].lines, g_files[i].bytes);
        buf_json_str(o, g_files[i].rel);
        buf_add(o, "}");
    }

    buf_add(o, "],\"syms\":[");
    for (int i = 0; i < g_nsyms; i++) {
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"id\":%d,\"file\":%d,\"kind\":\"%s\",\"line\":%d,\"end\":%d,\"name\":",
                 i, g_syms[i].file, SYMK[g_syms[i].kind], g_syms[i].line, g_syms[i].end);
        buf_json_str(o, g_syms[i].name);
        buf_add(o, "}");
    }

    buf_add(o, "],\"edges\":[");
    for (int i = 0; i < g_nfedges; i++) {
        FEdge *e = &g_fedges[i];
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"from\":%d,\"to\":%d,\"kind\":\"%s\",\"n\":%d,\"label\":",
                 e->from, e->to, KIND[e->kind], e->n);
        buf_json_str(o, e->label);
        buf_add(o, "}");
    }

    buf_add(o, "],\"calls\":[");
    for (int i = 0; i < g_ncalls; i++) {
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"from\":%d,\"to\":%d,\"kind\":\"%s\"}",
                 g_calls[i].from, g_calls[i].to, KIND[g_calls[i].kind]);
    }

    buf_add(o, "],\"unresolved\":[");
    for (int i = 0; i < g_nunres; i++) {
        if (i) buf_add(o, ",");
        buf_addf(o, "{\"file\":%d,\"line\":%d,\"kind\":\"%s\",\"target\":",
                 g_unres[i].file, g_unres[i].line, KIND[g_unres[i].kind]);
        buf_json_str(o, g_unres[i].target);
        buf_add(o, "}");
    }

    buf_addf(o, "],\"stats\":{\"files\":%d,\"syms\":%d,\"refs\":%d,\"edges\":%d,"
                "\"calls\":%d,\"unresolved\":%d,\"ms\":%d}}",
             g_nfiles, g_nsyms, g_nrefs, g_nfedges, g_ncalls, g_nunres, scan_ms());
}
