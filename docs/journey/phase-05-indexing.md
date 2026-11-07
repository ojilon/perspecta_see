# Phase 5: Function and include indexing (~4 weeks)

Heuristic function-definition finder, include graph, `index.json`. Compare
results against universal-ctags output to find your mistakes.

## Goal
`perspecta index <path>` walks a directory, tokenizes each `.c`/`.h` file,
heuristically finds function definitions, records `#include` relationships, and
writes `index.json`.

## Why it matters
Indexing is the core product: "where is this function defined?" and "which
files include this header?" Heuristics are enough to be useful, but you need an
independent check (universal-ctags) to learn where your heuristics break.

## Prerequisites
- Phase 4 definition of done (tokenizer).
- universal-ctags installed (optional but recommended — if you cannot install
  it, skip the comparison sessions and note it in the log).

## Sessions

### Session 1 — index command skeleton ([hand-written])
**Goal.** `perspecta index <path>` prints the path and counts files.
**Shape shown first.** Reuse the scanner's directory walk from Phase 3.
**Expected result.** `perspecta index src` prints how many `.c`/`.h` files found.
**Stop here.** Walk works; no function detection yet.

### Session 2 — index entry struct ([hand-written])
**Goal.** What one indexed symbol looks like.
**Shape shown first.**
```c
typedef struct {
    char kind[16];   /* "function" or "include" or "macro" */
    char name[260];
    char file[260];
    int line;
} IndexEntry;
```
**Expected result.** Struct compiles; array holds entries.
**Stop here.** Struct compiles.

### Session 3 — function-definition heuristic ([hand-written])
**Goal.** Spot `type name(args) {` at top level.
**Shape shown first.** Look for a TOK_PUNCT `{` whose previous non-whitespace
token is `)`, and the token before that is an identifier (the name), and before
that a type-like token. Require the `{` at column 0 or after only whitespace
(simple heuristic: top-level only).
**Expected result.** `src\main.c`'s `main` is found.
**Stop here.** One function detected in your own file.

### Session 4 — include detection ([hand-written])
**Goal.** From TOK_PREPROCESSOR tokens starting with `#include`, extract the file name.
**Shape shown first.** Token text `#include <stdio.h>` → name `stdio.h`, kind `include`.
**Expected result.** Includes of each file are listed.
**Stop here.** `#include "foo.h"` and `#include <foo.h>` both work.

### Session 5 — write index.json ([hand-written])
**Goal.** Emit all entries as json.
**Shape shown first.**
```c
printf("[\n");
for each entry: printf("  { \"kind\": \"%s\", \"name\": \"%s\", \"file\": \"%s\", \"line\": %d },\n");
printf("]\n");
```
**Expected result.** `index.json` created; opens in a text editor.
**Stop here.** Json written to file with `--output`.

### Session 6 — run ctags on your repo ([reading/tool])
**Goal.** Get ground truth to compare against.
**Shape shown first.** Run `ctags --output-format=json -f tags.json src\*.c src\*.h` (verify exact flags for your ctags version).
**Expected result.** `tags.json` exists with function entries.
**Stop here.** ctags output saved; comparison next session.

### Session 7 — compare your index vs ctags ([hand-written])
**Goal.** List functions ctags found that you missed, and vice versa.
**Expected result.** A short note: "I missed 3 static functions", "ctags found macros I skip", etc.
**Stop here.** Comparison written in session log; no code change required yet.

### Session 8 — fix one heuristic ([hand-written])
**Goal.** Act on one finding from session 7 (e.g. recognize `static` functions, or functions spanning multiple lines).
**Expected result.** One improvement in detection; re-run on your repo; count improves.
**Stop here.** Heuristic fixed and re-verified.

### Session 9 — reading session: RAD Debugger structure ([reading])
**Goal.** See how a real C project organizes files and functions.
**What to read.** RAD Debugger: one directory listing + one source file, 30 minutes, 5 notes (verify exact directory — look at the repo root first).
**Expected result.** 5 notes in session log.
**Stop here.** Notes written.

### Session 10 — index your own repos ([hand-written])
**Goal.** Dogfood: `perspecta index` on two of your own repos, inspect `index.json`.
**Expected result.** Two index files exist; wrong/missing entries noted.
**Stop here.** Tool used on real code.

### Session 11 — multiple files aggregation ([hand-written])
**Goal.** One `index.json` covering all files in the tree, each entry tagged with its file.
**Expected result.** Single json with entries from every `.c`/`.h`.
**Stop here.** Aggregated index works.

### Session 12 — warnings cleanup ([hand-written])
**Stop here.** Zero warnings.

### Session 13 — buffer / catch-up
**Stop here.** Sessions 1–12 complete.

### Session 14 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
`perspecta index <path> [--output index.json]` producing function + include
entries, compared against universal-ctags at least once.

## Definition of done
- [ ] Heuristic function-definition detection
- [ ] `#include` extraction
- [ ] `index.json` output (file option)
- [ ] Compared with universal-ctags once; gaps noted
- [ ] At least one heuristic improved from the comparison
- [ ] Run on own repos
- [ ] Clean build with warnings on

## What to read
- universal-ctags documentation (verify title/link) — output format and options.
- `docs/guides/studying-codebases.md` — note-taking method for the RAD Debugger session.

## Honest limit
Function detection is heuristic: it will miss complex signatures, macros that
look like functions, and function-like macros. That is acceptable in year one.
The ctags comparison tells you exactly how much you miss.

## Buffer
One buffer week before Phase 6.
