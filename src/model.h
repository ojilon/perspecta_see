#ifndef MODEL_H
#define MODEL_H
/* The "database" of one scan. Everything lives in fixed static arrays:
   no malloc bookkeeping, nothing big on the stack. */

enum { L_OTHER, L_C, L_H, L_JS, L_HTML, L_CSS };               /* languages */
enum { S_FUNC = 1, S_ROUTE, S_ID, S_CLASS };                   /* symbol kinds */
enum { R_INCLUDE = 1, R_SCRIPT, R_STYLE, R_IMPORT,             /* reference kinds */
       R_CLASS, R_DOM, R_API, R_CALL };

#define MAX_FILES  1500
#define MAX_SYMS   12000
#define MAX_REFS   20000
#define MAX_FEDGES 4000
#define MAX_CALLS  8000
#define MAX_UNRES  200
#define MAX_FILE_BYTES (1024 * 1024)

typedef struct { char rel[256]; int lang; int lines; long bytes; } File;
/* A named thing inside a file: a function, an /api route, an html id, a css class. */
typedef struct { int file; int kind; char name[64]; int line; int end; } Sym;
/* A mention found while parsing, resolved later: "this file points at <target>". */
typedef struct { int file; int from_sym; int kind; char target[128]; int line; } Ref;
/* File-to-file link (aggregated: n = how many refs say the same thing). */
typedef struct { int from, to, kind, n; char label[64]; } FEdge;
/* Symbol-to-symbol link, e.g. function A calls function B. */
typedef struct { int from, to, kind; } Call;
typedef struct { int file, kind, line; char target[128]; } Unres;

extern File  g_files[MAX_FILES];     extern int g_nfiles;
extern Sym   g_syms[MAX_SYMS];       extern int g_nsyms;
extern Ref   g_refs[MAX_REFS];       extern int g_nrefs;
extern FEdge g_fedges[MAX_FEDGES];   extern int g_nfedges;
extern Call  g_calls[MAX_CALLS];     extern int g_ncalls;
extern Unres g_unres[MAX_UNRES];     extern int g_nunres;

#endif