# Phase 6: Macros and basic call relations (~4 weeks)

List macros, rough call mapping (who-calls-whom), and release v0.1.0.

## Goal
Extend `index.json` with macro definitions and a call graph: for each
function, which other functions does it appear to call? Output stays textual;
precision stays heuristic.

## Why it matters
Call relations turn an index into a map: you can ask "who calls this function?"
before you understand the code. Macros are worth listing even if you do not
expand them — they explain constants and configuration in the code you read.

## Prerequisites
- Phase 5 definition of done (function + include index).

## Sessions

### Session 1 — macro detection ([hand-written])
**Goal.** Find `#define NAME ...` and `#define NAME(args) ...` lines.
**Shape son first.** From TOK_PREPROCESSOR tokens: if text starts with `#define`, the next identifier is the macro name; everything to end of line is the body (stored as text).
**Expected result.** `#define MAX 100` appears in index as a macro entry.
**Stop here.** Simple object-like macros detected.

### Session 2 — function-like macro detection ([hand-written])
**Goal.** Detect `(`, `)` after the name on the same `#define` line.
**Shape shown first.** After the macro name, if next non-space char is `(`, it is function-like; capture until end of line.
**Expected result.** `#define SQUARE(x) ((x) * (x))` detected as function-like.
**Stop here.** Both macro kinds detected.

### Session 3 — call-relation heuristic ([hand-written])
**Goal.** Inside a function body, `identifier(` suggests a call.
**Shape shown first.** For each `{...}` block assigned to a function, scan tokens: an identifier immediately followed by `(` is a candidate call to that identifier.
**Expected result.** `main` calling `printf` is recorded.
**Stop here.** One call relation recorded.

### Session 4 — who-calls-whom output ([hand-written])
**Goal.** Print, per function, the list of functions it calls.
**Shape shown first.**
```c
printf("%s calls: %s, %s\n", caller, callee1, callee2);
```
**Expected result.** `perspecta index` output includes call lists.
**Stop here.** Call lists print for your own repo.

### Session 5 — index.json gains macros and calls ([hand-written])
**Goal.** Add `macros` and `calls` arrays to the json.
**Shape shown first.** New json sections:
```json
"macros": [{"name": "MAX", "file": "...", "line": 3}],
"calls": [{"caller": "main", "callee": "printf"}]
```
**Expected result.** `index.json` contains macro and call entries.
**Stop here.** Json schema extended.

### Session 6 — reading session: macros in real code ([reading])
**Goal.** See how macros are used in a codebase you read.
**What to read.** One header of your own repo or RAD Debugger (verify path), 30 minutes, 5 notes on macro style (guards, constants, function-like macros).
**Expected result.** 5 notes in session log.
**Stop here.** Notes written.

### Session 7 — false-positive review ([hand-written])
**Goal.** Call heuristic mistakes: `sizeof(`, function pointers `(*fp)(`, casts `(type)(`.
**Shape shown first.** Exclude identifiers followed by `(` when the identifier is `sizeof` or aligned with known false patterns — keep it small, one rule at a time.
**Expected result.** Fewer obviously-wrong call entries.
**Stop here.** One false-positive rule added.

### Session 8 — run on Odin core ([hand-written])
**Goal.** Dogfood on a larger real codebase.
**What to read/run.** Odin source (vendor/ or core/ — verify exact path), 30 minutes reading + run indexer, 5 notes on what breaks.
**Expected result.** Indexer runs on Odin; breakages noted in log.
**Stop here.** Run complete; breakages recorded (fixes deferred to Phase 10 unless trivial).

### Session 9 — release v0.1.0 preparation ([hand-written])
**Goal.** Decide what v0.1.0 contains and write release notes.
**Shape shown first.** Checklist: scan, index, tokenize commands; txt/json output; `--version` flag.
**Expected result.** Release notes drafted in session log.
**Stop here.** Scope of v0.1.0 agreed in writing.

### Session 10 — `--version` flag ([hand-written])
**Goal.** `perspecta --version` prints the version.
**Shape shown first.** `#define PERSPECTA_VERSION "0.1.0"` in a shared header; print it early in `main`.
**Expected result.** `perspecta.exe --version` prints `perspecta_see 0.1.0`.
**Stop here.** Version embedded and visible.

### Session 11 — warnings cleanup + tag ([hand-written])
**Stop here.** Zero warnings; create git tag `v0.1.0`.

### Session 12 — buffer / catch-up
**Stop here.** Sessions 1–11 complete.

### Session 13 — phase checkpoint + release
**Stop here.** Checklist below complete; tag `v0.1.0` created.

## Deliverable
`perspecta index` output includes macros and call relations; `perspecta
--version` prints `0.1.0`; git tag `v0.1.0`.

## Definition of done
- [ ] Object-like and function-like macro detection
- [ ] Call relation heuristic (who-calls-whom)
- [ ] `index.json` has `macros` and `calls` sections
- [ ] One false-positive rule added
- [ ] Ran on Odin source; breakages noted
- [ ] `--version` flag works
- [ ] Git tag `v0.1.0`
- [ ] Clean build with warnings on

## What to read
- `docs/guides/releases-versioning.md` — version meaning, release checklist.
- `docs/guides/config.md` — CLI flag conventions (`--version` added here).

## Honest limit
Call detection is syntactic: `func_ptr()` and macro names look like calls.
Macros are not expanded. False positives are expected and noted, not fixed to
perfection this year.

## Buffer
One buffer week before Phase 7.
