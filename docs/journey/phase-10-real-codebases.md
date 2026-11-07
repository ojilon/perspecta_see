# Phase 10: Reading real codebases (~4 weeks)

Run the tool on the RAD Debugger, Odin core/vendor, and your local GCC copy.
Record what breaks and fix the tokenizer and indexer. GCC study begins here:
function search and indexing, how macros are expanded, how the preprocessor
works at a high level.

## Goal
Stress-test perspecta_see on real, large C codebases. Fix what breaks in the
tokenizer/indexer. Begin studying GCC: how it searches functions, indexes them,
expands macros, and runs the preprocessor — at a high level only (GCC is huge;
you are learning its shape, not reimplementing it).

## Why it matters
Your own toys are small. Real codebases have weird macros, deep includes,
multi-line macros, and preprocessor tricks. This phase teaches where your
heuristics break and how real C is actually written.

## Prerequisites
- Phase 9 definition of done (code generation).
- Local copies: RAD Debugger repo, Odin source, GCC source (verify locations).

## Sessions

### Session 1 — run on RAD Debugger ([hand-written])
**Goal.** `perspecta scan` + `perspecta index` on the RAD Debugger repo; note crashes and wrong counts.
**Shape shown first.** Reuse scanner/indexer; run on the repo root (verify exact path).
**Expected result.** Tool runs (or crashes); every crash logged with file and command.
**Stop here.** First run complete; breakages listed.

### Session 2 — reading session: RAD Debugger base layer ([reading])
**Goal.** Read the RAD Debugger's base layer: memory, strings, error handling.
**What to read.** One base-layer file (verify path — look for `base/` or `src/base/`), 30 minutes, 5 notes on naming, ownership, error style.
**Expected result.** 5 notes; compare with your `arena.h`/`error.h`.
**Stop here.** Notes written.

### Session 3 — fix tokenizer breakages from RAD Debugger ([hand-written])
**Goal.** Fix one tokenizer issue found in session 1 (e.g. long strings, weird comments, wide characters `L"..."`).
**Expected result.** One fix; re-run on RAD Debugger; fewer crashes.
**Stop here.** One issue fixed.

### Session 4 — run on Odin core ([hand-written])
**Goal.** `perspecta index` on Odin core/vendor; note what breaks (verify path).
**Expected result.** Runs or crashes; breakages logged.
**Stop here.** Run complete; breakages listed.

### Session 5 — reading session: Odin source ([reading])
**Goal.** Read one Odin source file; 30 minutes, 5 notes on style (naming, types, macros).
**Expected result.** 5 notes in session log.
**Stop here.** Notes written.

### Session 6 — fix indexer breakages from Odin ([hand-written])
**Goal.** Fix one indexing issue (e.g. includes with relative paths, multi-line defines).
**Expected result.** Re-run on Odin; more entries correct.
**Stop here.** One issue fixed.

### Session 7 — GCC study: locate function definitions ([reading])
**Goal.** In your local GCC copy, find where functions are defined and how the build names them.
**What to read.** GCC: one top-level directory + one source file (verify path — look at gcc/ first), 30 minutes, 5 notes.
**Expected result.** 5 notes: how GCC organizes code, where `main`-like entry points live.
**Stop here.** Notes written.

### Session 8 — GCC study: macros and preprocessor at high level ([reading])
**Goal.** Understand how GCC uses the preprocessor: `#include` chains, config macros, generated headers.
**What to read.** GCC manual preprocessor section (verify title/link) + one GCC header with heavy `#if` usage, 30 minutes, 5 notes.
**Expected result.** 5 notes on GCC preprocessing style.
**Stop here.** Notes written.

### Session 9 — run on GCC subset ([hand-written])
**Goal.** Run scanner/indexer on one GCC directory (not the whole tree — too big for one session).
**Expected result.** Runs on a subset; performance or breakages noted.
**Stop here.** Subset run complete.

### Session 10 — fix one GCC-related breakage ([hand-written])
**Goal.** Fix one issue found on GCC (e.g. very long lines, system headers).
**Expected result.** Fix applied; re-run on subset.
**Stop here.** One fix applied.

### Session 11 — reading session: your notes review ([reading])
**Goal.** Re-read all Phase 10 notes; write one page: "what real C looks like vs what my tool assumes."
**Expected result.** Summary note in session log.
**Stop here.** Summary written.

### Session 12 — warnings cleanup ([hand-written])
**Stop here.** Zero warnings.

### Session 13 — buffer / catch-up
**Stop here.** Sessions 1–12 complete.

### Session 14 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
Tool runs on RAD Debugger, Odin (subset), GCC (subset); breakages logged and
several fixed; GCC study notes written.

## Definition of done
- [ ] Ran on RAD Debugger; breakages logged
- [ ] At least one RAD Debugger tokenizer/indexer fix
- [ ] Ran on Odin; breakages logged
- [ ] At least one Odin fix
- [ ] GCC study notes: function search, indexing, macro expansion, preprocessor (high level)
- [ ] Ran on a GCC subset; breakages noted
- [ ] Clean build with warnings on

## What to read
- GCC manual: preprocessor and warnings sections (verify title/link).
- universal-ctags documentation (verify title/link) — still the comparison baseline.
- RAD Debugger base layer (one file).
- Odin source (one file).
- `docs/guides/studying-codebases.md` — note-taking method.

## Honest limit
GCC is enormous. You are not parsing GCC fully; you are running your tool on a
subset and reading its structure. Full GCC support is a year-2 topic at the
earliest.

## Buffer
One buffer week before Phase 11.
