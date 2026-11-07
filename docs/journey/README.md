# perspecta_see — one-year learning and building journey

perspecta_see is being rebuilt from the ground up as a CLI-first tool in C:
read a C codebase, index it, map it (files, functions, includes, macros, call
relationships), output reports as txt and json, and illustrate structure and
flow with ASCII art. It should be useful while it is still being built, not
only at the end.

This folder holds the one-year plan: 12 phases, daily sessions of 15–45
minutes, session logs, reading lists, and the rules that keep the project
moving on a Celeron PC with Termux on Android, through exams and power cuts.

## Where the code is today

- `src/error.h`, `src/error.c` — `Error` struct, `ERROR`/`ERROR_OK` macros,
  `error_print`. Done.
- `src/file.h`, `src/file.c` — `File` handle wrapping `FILE*`, `File_open`,
  `File_close`. Reading whole files and reading in chunks comes in Phase 1.
- `src/main.c` — takes a path, prints path, size, line count. It still opens
  files with raw `fopen`; Phase 1 moves it onto the `File` API.

Nothing in `docs/` changes the C code. The plan describes what to write next.

## The phases

| Phase | Focus | Rough length |
| ----- | ----- | ------------ |
| 1 | Foundation I: error + file APIs, integer typedefs, build script | ~4 weeks |
| 2 | Foundation II: arenas, strings, dynamic arrays | ~4 weeks |
| 3 | Scanner: walk directories, txt + json project summary | ~4 weeks |
| 4 | Tokenizer: C tokens, comments, preprocessor lines | ~4 weeks |
| 5 | Function + include indexing, `index.json` | ~4 weeks |
| 6 | Macros + basic call relations, release v0.1.0 | ~4 weeks |
| 7 | ASCII views: trees, include graph, call graph, report command | ~3 weeks |
| 8 | Scopes + diagnostics | ~4 weeks |
| 9 | Code generation: enum/struct tables, project builds part of itself | ~3 weeks |
| 10 | Reading real codebases: RAD Debugger, Odin, GCC | ~4 weeks |
| 11 | Hardening: speed on low-end hardware, errors, tests, docs | ~3 weeks |
| 12 | Stabilization: v1.0.0 decisions, retrospective, year-2 plan | ~3 weeks |

Buffer weeks sit between phases. Skipping a week is allowed: the plan slides,
it does not break. Re-read your last session log and continue.

## How to use these docs

1. Pick the current phase file (start with `phase-01-foundation-i.md`).
2. Do one numbered session per sitting. Sessions are sized for 20–30 minutes;
   on 15-minute days do half a session and stop at a compiling state.
3. Every session must end compiling, with one small, visible result.
4. Log the session in `docs/journey/sessions/` (see SESSION_TEMPLATE.md).
5. When a phase's definition of done is met, move to the next phase.

## Rules of the journey

1. **Every session ends compiling.** If a feature is not finished, revert to
   the last compiling state and log that.
2. **One concept per session.** Do not start arenas and strings on the same day.
3. **Hand-write the core.** AI may explain, review, and generate test inputs
   only. The parser, indexer, and generators are written by me, line by line.
   Sessions that must be hand-written are marked `[hand-written]`.
4. **No new feature before the current phase's definition of done.**
5. **Ideas go to `docs/ideas/BACKLOG.md`, not into today's session.**
6. **Log every session** in one or two lines: date, what I did, what I learned,
   what is next.
7. **Missing days is fine.** Re-read the last log entry and continue.
8. **Dogfood early.** Run the tool on real code from Phase 3 (your own repos
   first, RAD Debugger and Odin later).

## Assumptions

- Builds with plain `gcc` only, plus an optional tiny shell/bat build script or
  CMake. No network needed to build.
- Hardware: Windows PC (Celeron) + Termux on Android. Examples use Windows
  paths (`src\main.c`) but commands work on both.
- Sessions: 15–45 minutes, most days.
- "Parsing C" in year one means heuristics: no full preprocessing, no macro
  expansion engine, no full type resolution. That is deliberate — heuristics
  are enough to be useful, and they keep every session finishable.
- This is a hobby alongside a university degree. The plan tolerates gaps.

## What to read next

- `docs/journey/phase-01-foundation-i.md` — start here.
- `docs/guides/code-style.md` — naming, headers, ownership before writing much
  new code.
- `docs/guides/error-handling.md` — how errors flow through the code.
