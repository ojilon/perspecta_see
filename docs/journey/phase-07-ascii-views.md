# Phase 7: ASCII views (~3 weeks)

Tree views, include graph and call graph in ASCII, and a `report` command.

## Goal
`perspecta report <path>` prints: a file tree, an include graph (file → files it
includes), and a call graph (function → functions it calls), all as ASCII art
suitable for a terminal.

## Why it matters
Maps are easier to understand than lists. ASCII art works everywhere (Celeron,
Termux, over SSH) and needs no graphics library. It is the "illustrations of
structure and flow" from the project goal, in the simplest possible form.

## Prerequisites
- Phase 6 definition of done (index with macros and calls).

## Sessions

### Session 1 — report command skeleton ([hand-written])
**Goal.** `perspecta report <path>` prints the path.
**Shape shown first.** Reuse the directory walk from Phase 3.
**Expected result.** `perspecta report src` prints `report: src`.
**Stop here.** Command parses.

### Session 2 — file tree in ASCII ([hand-written])
**Goal.** Print directories with indentation and files as leaves.
**Shape son first.** Recursive walk: print `  dir/\n    file.c\n` using depth-based spaces.
**Expected result.** Indented tree of `src\` prints on screen.
**Stop here.** Tree view works for one directory depth.

### Session 3 — deeper tree ([hand-written])
**Goal.** Multiple levels with `├──` / `└──` style connectors (keep it simple: plain indentation is acceptable).
**Expected result.** Nested tree prints correctly.
**Stop here.** Tree view shows 2+ levels.

### Session 4 — include graph in ASCII ([hand-written])
**Goal.** From `index.json` include entries, print `file.c -> included.h`.
**Shape shown first.** One line per edge, sorted by file.
**Expected result.** Graph prints for your own repo.
**Stop here.** Include edges visible.

### Session 5 — include graph as tree ([hand-written])
**Goal.** Root at a chosen file, show its includes recursively, indentation = depth, cycle guard (stop revisiting).
**Shape shown first.** Track visited files in an array; if already visited, print `(cycle)` and stop.
**Expected result.** No infinite loop on circular includes.
**Stop here.** Cycle-safe include tree.

### Session 6 — call graph in ASCII ([hand-written])
**Goal.** From call entries, print `caller -> callee`.
**Expected result.** Call edges print; functions with no calls print `name -> (none)`.
**Stop here.** Call graph visible.

### Session 7 — `report` combines all three ([hand-written])
**Goal.** One command prints tree, include graph, call graph with section headers.
**Shape shown first.** `printf("\n=== FILE TREE ===\n");` etc.
**Expected result.** `perspecta report src` prints all three sections.
**Stop here.** Full report runs end to end.

### Session 8 — reading session: ASCII in the wild ([reading])
**Goal.** See how another tool draws graphs in text.
**What to read.** Any README or source of a tool with ASCII output (ctags, a dependency graph tool — verify what you have locally), 30 minutes, 5 notes on layout style.
**Expected result.** 5 notes; maybe one layout idea copied.
**Stop here.** Notes written.

### Session 9 — report on your own repos ([hand-written])
**Goal.** Dogfood: `perspecta report` on two of your own repos.
**Expected result.** Reports exist; unreadable sections noted.
**Stop here.** Used on real code.

### Session 10 — warnings cleanup ([hand-written])
**Stop here.** Zero warnings.

### Session 11 — buffer / catch-up
**Stop here.** Sessions 1–10 complete.

### Session 12 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
`perspecta report <path>` prints file tree + include graph + call graph as ASCII.

## Definition of done
- [ ] File tree ASCII view
- [ ] Include graph ASCII view (cycle-safe)
- [ ] Call graph ASCII view
- [ ] `report` command combines all three
- [ ] Run on own repos
- [ ] Clean build with warnings on

## What to read
- `docs/guides/studying-codebases.md` — for the ASCII-reading session.

## Honest limit
Graphs are ASCII text; large codebases produce wide output. That is fine — the
point is understanding, not pretty printing. "Better approaches later" means
Phase 11+ or year 2.

## Buffer
One buffer week before Phase 8.
