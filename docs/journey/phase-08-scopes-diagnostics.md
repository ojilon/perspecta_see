# Phase 8: Scopes and diagnostics (~4 weeks)

Simple scope tracking, catching simple issues (unclosed braces, unused statics,
missing includes, suspicious patterns), compared with cppcheck results.

## Goal
`perspecta check <file.c>` reports simple problems: braces that never close,
static functions never called, `#include` lines that look unused, and obvious
suspicious patterns (e.g. `=` inside `if (` — keep the list tiny).

## Why it matters
Diagnostics turn the tool from "map the code" into "help me understand the
code." Catching even a few real issues in your own code proves value. cppcheck
is the independent benchmark: compare, note gaps, do not try to match it.

## Prerequisites
- Phase 7 definition of done (report command).
- cppcheck installed (optional — if unavailable, skip comparison sessions).

## Sessions

### Session 1 — check command skeleton ([hand-written])
**Goal.** `perspecta check <file.c>` tokenizes and prints "checked".
**Expected result.** `perspecta check src\main.c` prints `checked: src\main.c`.
**Stop here.** Command parses and tokenizes.

### Session 2 — brace balance ([hand-written])
**Goal.** Count `{` and `}` outside strings and comments; report mismatch.
**Shape shown first.** While tokenizing, increment on `{`, decrement on `}` (skip tokens inside TOK_STRING/TOK_COMMENT); final count != 0 → error.
**Expected result.** A file with one missing `}` prints `error: unclosed brace`.
**Stop here.** Brace check works on a deliberately broken file.

### Session 3 — static function usage ([hand-written])
**Goal.** From `index.json` call data, list `static` functions never called.
**Shape shown first.** Function entry has `static` flag (from tokenizer seeing `static` before the name); if name never appears in any `calls` entry → report.
**Expected result.** Unused statics in your own repo listed.
**Stop here.** At least one unused static found or none reported honestly.

### Session 4 — suspicious pattern: assignment in condition ([hand-written])
**Goal.** Flag `if (` followed later by `=` before `)` — keep it one rule.
**Shape shown first.** Inside `if (...)`, scan tokens; `TOK_PUNCT` `=` not `==` → warning.
**Expected result.** `if (x = 5)` flagged; `if (x == 5)` not.
**Stop here.** One rule, tested on a deliberately broken file.

### Session 5 — `check` output format ([hand-written])
**Goal.** One line per issue: `file:line: kind: message`.
**Shape shown first.** `printf("%s:%d: warning: %s\n", file, line, kind, message);`
**Expected result.** Issues print in a grep-friendly format.
**Stop here.** Format stable across sessions.

### Session 6 — cppcheck on your repo ([reading/tool])
**Goal.** Ground truth for diagnostics.
**Shape shown first.** Run `cppcheck --enable=all src\` (verify flags) and save output.
**Expected result.** cppcheck output file exists.
**Stop here.** cppcheck output saved; comparison next session.

### Session 7 — compare your check vs cppcheck ([hand-written])
**Goal.** What does cppcheck find that you miss? What do you find it misses?
**Expected result.** Short note in session log.
**Stop here.** Comparison written.

### Session 8 — reading session: RAD Debugger diagnostics ([reading])
**Goal.** See how a real C project structures its own checks or asserts.
**What to read.** RAD Debugger (verify directory), 30 minutes, 5 notes on error/assert style.
**Expected result.** 5 notes in session log.
**Stop here.** Notes written.

### Session 9 — check on your own repos ([hand-written])
**Goal.** Dogfood: `perspecta check` on two of your own repos.
**Expected result.** Reports exist; false positives noted.
**Stop here.** Used on real code.

### Session 10 — warnings cleanup ([hand-written])
**Stop here.** Zero warnings.

### Session 11 — buffer / catch-up
**Stop here.** Sessions 1–9 complete.

### Session 12 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
`perspecta check <file.c>` prints brace/static/include/suspicious issues in
`file:line: kind: message` format; compared with cppcheck once.

## Definition of done
- [ ] Brace balance check
- [ ] Unused static function check
- [ ] One suspicious-pattern check (assignment in condition)
- [ ] Output format `file:line: kind: message`
- [ ] Compared with cppcheck once; gaps noted
- [ ] Run on own repos
- [ ] Clean build with warnings on

## What to read
- cppcheck documentation (verify title/link) — what it checks, how to run it.
- `docs/guides/studying-codebases.md` — note-taking method.

## Honest limit
This is a linter-lite, not a real analyzer. No type resolution, no control
flow beyond braces. cppcheck will find far more — that is the point of the
comparison: know what your tool does not do.

## Buffer
One buffer week before Phase 9.
