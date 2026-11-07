# Phase 12: Stabilization (~3 weeks)

Cleanup, v1.0.0 decisions, retrospective, and a plan for the second year.

## Goal
Decide what v1.0.0 means for perspecta_see, cut what does not fit, write the
retrospective, and sketch year 2.

## Why it matters
Without a stop condition the project keeps growing forever. A v1.0.0 decision
forces you to say what is core and what is a future idea (that goes to
BACKLOG.md).

## Prerequisites
- Phase 11 definition of done.

## Sessions

### Session 1 — feature freeze list ([hand-written])
**Goal.** List every command and flag; mark core vs future.
**Shape shown first.** Table: command | core for v1.0.0? | note.
**Expected result.** Written list in session log.
**Stop here.** List complete.

### Session 2 — cleanup ([hand-written])
**Goal.** Remove dead code, unused functions, leftover debug prints.
**Expected result.** Fewer warnings, smaller source.
**Stop here.** Clean compile.

### Session 3 — final warnings audit ([hand-written])
**Stop here.** Zero warnings with `-Wall -Wextra -Wpedantic -Werror`.

### Session 4 — v1.0.0 release notes ([hand-written])
**Goal.** Write what v1.0.0 ships and what it explicitly does not.
**Expected result.** Release notes drafted.
**Stop here.** Notes complete.

### Session 5 — tag v1.0.0 ([hand-written])
**Stop here.** Git tag `v1.0.0` created.

### Session 6 — reading session: your year ([reading])
**Goal.** Re-read session logs from Phase 1; write what you learned about C.
**Expected result.** One page of lessons in session log.
**Stop here.** Retrospective written.

### Session 7 — year-2 plan ([hand-written])
**Goal.** Sketch 6–8 phase ideas for year 2 (full preprocessor? type resolution?
better graphs? language server?). All go to BACKLOG.md until year 2 starts.
**Expected result.** Year-2 plan written.
**Stop here.** Plan complete.

### Session 8 — buffer / catch-up
**Stop here.** Sessions 1–7 complete.

### Session 9 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
v1.0.0 tag, release notes, retrospective, year-2 plan in BACKLOG.md.

## Definition of done
- [ ] Feature freeze list written
- [ ] Cleanup done; zero warnings
- [ ] v1.0.0 release notes written
- [ ] Git tag `v1.0.0`
- [ ] Retrospective written
- [ ] Year-2 plan in BACKLOG.md

## What 1.0.0 might mean (decide in Session 1)
- `scan`, `index`, `tokenize`, `report`, `check`, `gen` commands work
- txt + json output
- ASCII views
- Explicitly NOT: full C preprocessing, type resolution, speed for million-line trees

## Buffer
The year ends here. If you miss weeks, the plan slides — finish the phases you
can and carry the rest into year 2.
