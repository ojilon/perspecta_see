#ifndef MODEL_H
#define MODEL_H

/* model.h — the "database" of one scan.
 *
 * Everything a scan learns is written into the fixed arrays declared here and
 * defined in src/store.c. Nothing is malloc'd per file, so a scan can be run
 * again and again without leaking: store_reset() just sets the counters to 0.
 *
 *   File  — one file on disk (path relative to the scanned root, language, size)
 *   Sym   — a named thing *defined* in a file: a function, an /api route,
 *           an html id, a css class. `line`..`end` is its scope.
 *   Ref   — a *mention* found while parsing: "this file points at <target>".
 *           Refs are what make the interesting links; they are resolved after
 *           every file has been read (see src/resolve.c).
 *   FEdge — file -> file link, aggregated: n says how many refs agree.
 *   Call  — symbol -> symbol link, e.g. function A calls function B.
 *   Unres — a ref that matched nothing. This is the "dangling links" list the
 *           browser shows; it is also the best tool for improving the parser.
 */

enum { L_OTHER, L_C, L_H, L_JS, L_HTML, L_CSS };                 /* languages */
enum { S_FUNC = 1, S_ROUTE, S_ID, S_CLASS };                     /* symbol kinds */

/* reference kinds — the order here must match KIND[] in src/store.c and the
   CSS colours in web/js/state.js */
enum { R_INCLUDE = 1, R_SCRIPT, R_STYLE, R_IMPORT,
       R_CLASS, R_DOM, R_API, R_CALL, R_HREF, R_MCALL };

/* Fixed ceilings. Bigger projects simply stop being recorded (never crash). */
#define MAX_FILES  1500
#define MAX_SYMS   12000
#define MAX_REFS   20000
#define MAX_FEDGES 4000
#define MAX_CALLS  8000
#define MAX_UNRES  200
#define MAX_FILE_BYTES (1024 * 1024)

typedef struct { char rel[256]; int lang; int lines; long bytes; } File;

typedef struct {
    int file;            /* index into g_files */
    int kind;            /* S_* */
    char name[64];
    int line;            /* first line of the definition */
    int end;             /* last line of the body (== line for one-liners) */
} Sym;

typedef struct {
    int file;            /* who mentions it */
    int from_sym;        /* which function the mention sits in, or -1 */
    int kind;            /* R_* */
    char target[128];    /* raw text: "js/app.js", "tree", ".row", "/api/scan" */
    int line;
} Ref;

typedef struct { int from, to, kind, n; char label[64]; } FEdge;
typedef struct { int from, to, kind; } Call;
typedef struct { int file, kind, line; char target[128]; } Unres;

extern File  g_files[MAX_FILES];     extern int g_nfiles;
extern Sym   g_syms[MAX_SYMS];       extern int g_nsyms;
extern Ref   g_refs[MAX_REFS];       extern int g_nrefs;
extern FEdge g_fedges[MAX_FEDGES];   extern int g_nfedges;
extern Call  g_calls[MAX_CALLS];     extern int g_ncalls;
extern Unres g_unres[MAX_UNRES];     extern int g_nunres;

/* C and C++ headers count as one family, JavaScript as another. Links are
   never resolved across families. */
int lang_family(int lang);

#endif
