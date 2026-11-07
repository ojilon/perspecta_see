# Phase 11: Hardening (~3 weeks)

Performance on large trees on a low-end machine, error messages, tests, docs.

## Goal
Make perspecta_see fast enough to scan a large tree on a Celeron, make errors
clear and complete, add enough tests, and write the docs that let you resume
after a gap.

## Why it matters
A tool that takes minutes on your codebase is a tool you will not use. Hardening
is what makes it usable day to day while you keep building.

## Prerequisites
- Phase 10 definition of done.

## Sessions

### Session 1 — measure current speed ([hand-written])
**Goal.** Time `perspecta scan` on your largest repo.
**Shape shown first.** Run the scan with a stopwatch; note seconds and file count.
**Expected result.** A baseline number in the session log (e.g. "12s for 80 files").
**Stop here.** Baseline recorded.

### Session 2 — avoid re-reading files ([hand-written])
**Goal.** The indexer tokenizes each file once; scan does not open files twice.
**Shape shown first.** Review `main.c` scan path for duplicate `File_open` calls; remove them.
**Expected result.** Same output, fewer file opens.
**Stop here.** Duplicate open removed.

### Session 3 — bigger read chunks ([hand-written])
**Goal.** `File_read_whole` reads in larger chunks instead of byte-by-byte.
**Shape shown first.** `fread` into a buffer of 4096 bytes in a loop until EOF.
**Expected result.** Compiles; read speed improves.
**Stop here.** Chunked whole-file read implemented.

### Session 4 — time again ([hand-written])
**Goal.** Re-measure on the same repo; compare with baseline.
**Expected result.** Faster number in the log (or "no change — bottleneck elsewhere").
**Stop here.** Measurement recorded.

### Session 5 — error message audit ([hand-written])
**Goal.** Every error says what happened, where, and the system cause (errno).
**Shape shown first.** Grep for `ERROR(` in `src/`; check each has a clear message and `system_code` where relevant.
**Expected result.** List of messages; unclear ones rewritten.
**Stop here.** All errors pass the "what/where/cause" test.

### Session 6 — tests for scanner + tokenizer ([hand-written])
**Goal.** Small test programs: `tests/scan_test.c`, `tests/token_test.c`.
**Shape shown first.** `main` calls the function, checks counts, prints PASS/FAIL.
**Expected result.** Tests compile and run; failures visible.
**Stop here.** Two test files pass.

### Session 7 — tests for indexer ([hand-written])
**Goal.** `tests/index_test.c`: check function count on a tiny fixture file.
**Expected result.** Test passes; fixture file committed alongside.
**Stop here.** Indexer test passes.

### Session 8 — reading session: test style in C ([reading])
**Goal.** Look at how another small C project tests (your repos, RAD Debugger tests if any — verify), 30 minutes, 5 notes.
**Expected result.** 5 notes; maybe one testing habit copied.
**Stop here.** Notes written.

### Session 9 — run all tests ([hand-written])
**Goal.** One command runs every test program.
**Shape shown first.** `build.bat` gains a `test` label, or a `run_tests.bat` compiles and runs all `tests/*.c`.
**Expected result.** `./run_tests` prints PASS for every test.
**Stop here.** All tests green.

### Session 10 — docs pass: README + guides updated ([hand-written])
**Goal.** Update root `README.md` and any stale guide with what the tool does now.
**Expected result.** Docs match current commands (`scan`, `index`, `tokenize`, `report`, `check`, `gen`).
**Stop here.** Docs consistent with code.

### Session 11 — warnings cleanup ([hand-written])
**Stop here.** Zero warnings.

### Session 12 — buffer / catch-up
**Stop here.** Sessions 1–11 complete.

### Session 13 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
Faster scans, complete error messages, test suite with a one-command runner,
docs matching the code.

## Definition of done
- [ ] Baseline speed measured; improved by at least one change
- [ ] No duplicate file opens in scan/index paths
- [ ] All errors carry what/where/cause
- [ ] Test programs for scanner, tokenizer, indexer; one-command runner
- [ ] All tests pass
- [ ] Docs updated to current commands
- [ ] Clean build with warnings on

## What to read
- `docs/guides/releases-versioning.md` — release checklist (used after this phase).
- `docs/guides/error-handling.md` — error message audit reference.

## Buffer
One buffer week before Phase 12.
