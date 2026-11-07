# Practical addendum: from code reader to tool-building foundation

This is a **proposed companion**, not a rewrite of the journey. Keep the
[12 phases](README.md), their ideas, and the guides. Where a specific example
or checkpoint below conflicts with an older one, use this addendum as the safer
interpretation; update the original documents later when you choose. The goal
is not to make every idea a year-one requirement.

## The direction

```text
read one file -> scan a project -> find and inspect evidence ->
share a small foundation -> generate a useful table ->
optional experiments in a tool-building environment
```

The foundation is a **toolbox grown from real needs**, not an attempt to copy
another project's entire base layer. Study C and other systems languages for
ideas about ownership, data layout, explicit failure, tooling and developer
experience; translate a small idea into C only after understanding its
trade-offs. Hand-write the core. AI can explain, review, and generate test
inputs, but should not write the core implementation for you. Existing reading,
notes and experiments count as progress.

## What is actually here now

I read `src/error.h`, `src/error.c`, `src/file.h`, `src/file.c`, `src/main.c`
and the journey and guides. In this checkout, `gcc -std=c11 -Wall -Wextra
-Wpedantic src/*.c` built without diagnostics, and running it on `src/main.c`
printed a path, size and line count. That is one local compiler check, not a
Windows or Termux test. There is no build script or test suite in this checkout.

- `Error` has a code, literal message, optional system code and **implementation
  source** file/line; `error_print` prints it. `File_open` and `File_close`
  exist. There are no whole-file or chunk-read functions yet.
- Contrary to the journey overview, `Count_lines` already opens and closes via
  `File_open`/`File_close`; it reads directly from `fileobject.handle` with
  `fgetc`. Replace the direct read gradually, not the working CLI wholesale.
- `File_open` resets the object before opening. Calling it on an already-open
  object loses the old handle. `File.path` borrows the caller's pointer; its
  lifetime must cover use. The public handle and `is_open` can disagree if
  callers manipulate them. Decide and document lifecycle invariants before
  trying to hide implementation details.
- `Count_lines` assigns the count and size before close succeeds, so a close
  error can accompany changed output. `int`/`long` counters may overflow on
  large inputs; the fixed 260-byte path buffer rejects longer paths. These are
  real limits, not a reason to rewrite everything today.
- `ERROR`'s file/line identifies where an error was created, **not** the input
  path or location. `errno` is useful when the failing operation actually
  supplies it; don't promise a meaningful system cause for every failure.
- The style guide calls functions PascalCase but gives both `File_open` and
  `error_print` as examples. Prefer `Type_operation` for type-associated
  operations and `lower_snake_case` for other functions, matching existing
  code; no mass rename. Treat this as a proposal to reconcile the guide.
- The reading guide says Beej is in the repo, but no copy is present in this
  checkout. Use your own copy if available; don't block work on finding it.

## Next small steps: phase 1, without a rewrite

Each checkbox can be a separate 15–30 minute session. End with a compiling
program, a visible check, and a one- or two-line session log. If a step is too
large, split it; temporarily leave an unused declaration rather than an unsafe
implementation. Read the relevant guide alongside the steps.

1. **[hand-written] Baseline.** Example: `perspecta src/main.c`. Write down
   the command and observed output; also try a missing file. Stop when the
   successful and failure cases are recorded.
2. **[hand-written] Lifecycle contract.** Example: `File f = {0};` means closed;
   `File_open(&f, path, "rb")` requires a closed object. Document what happens
   on failed open, repeated close and attempted reopen, and that `path` is
   borrowed. Stop when a reader can identify who closes the handle.
3. **[hand-written] Lifecycle checks.** Example cases: open a real file, close
   twice, open a missing file, attempt to open an already-open object. Check
   that failure doesn't leak or destroy an existing handle. Stop when a tiny
   test reports pass/fail. Avoid inspecting uninitialized `File` fields: make
   initialization a caller precondition and initialize every call site.
4. **[hand-written] Read chunks first.** Shape: `Error File_read_chunk(File *f,
   void *buffer, size_t capacity, size_t *out_read);`. Specify valid arguments,
   whether zero capacity is allowed, EOF as success with zero bytes, and output
   state on failure. Add one fixture and test empty, short, exact-chunk and
   multi-chunk input. Stop after one passing case if needed.
5. **[hand-written] Use chunks for stats.** Example fixture: empty -> 0 lines;
   `a\n` -> 1; `a\nb` -> 2. Define lines as newline count plus one for a
   nonempty unterminated final line. Check bytes, CRLF and no final newline.
   Handle overflow explicitly; publish results only after successful read and
   close. If read and close both fail, preserve the read error as primary and
   document how the close failure is reported. Stop when existing CLI output
   still works and one fixture passes.
6. **[hand-written] Whole-file read only when needed.** Specify ownership and
   whether a trailing NUL is included; initialize outputs on failure, handle
   empty files, allocation/size overflow and partial reads. Prefer a checked
   chunk loop if non-seekable input matters; if choosing `fseek`/`ftell`,
   explicitly restrict it to seekable files and check every result. Caller
   frees the returned heap buffer. Stop with one successful test, then add
   failure tests in separate sessions.
7. **[hand-written] Build and test entry points.** Keep a tiny `.bat` and `.sh`
   with equivalent gcc commands, or document direct commands on each system.
   Run the CLI and small test executables offline. Check paths with spaces.
   Stop once one command builds on the device currently available; record the
   other platform as unverified until actually tested.
8. **[reading] One file, five notes.** Read the local `src/file.c`, then the
   relevant file I/O section of your C reference: contract, ownership, failure,
   one surprise, one question. Stop after five notes; the existing build still
   works.

**Important correction:** the Phase 1 whole-file sketch performs unchecked
seek, allocation and read operations, and its chunk example captures `errno`
but does not put it into the error. Treat those snippets as incomplete shapes,
not safe code to run as written. `malloc(0)` returning NULL need not mean an
allocation failure. Output invariants and cleanup matter before a demo uses
these APIs.

## Build the foundation only when it earns its place

| Stage | Need and hand-written abstraction | Contract and smallest proof |
| --- | --- | --- |
| 1 | `Error`, file handle, fixed-width types | Explicit borrowed/owned data, initialization, cleanup, overflow and errors; tiny tests. Exact-width `stdint.h` types are conditional on platform support; `usize` is `size_t`. |
| 2 | Arena for temporary scan data | Explicit backing storage/ownership, alignment, checked capacity arithmetic, reset invalidates all arena pointers; test alignment and exhaustion. |
| 2 | String view and vector for paths/tokens | View is pointer + length, not necessarily NUL-terminated; owner outlives view. Vector growth checks overflow/allocation failure; define who owns elements and backing memory. |
| 3–6 | Directory and source-location APIs | Isolate POSIX and Windows behind a small interface; test end-of-directory separately from error. Store file path plus byte offset and line; keep source alive while views point into it. |
| 9 | Generator and shared records | Use a small explicit input format; deterministic output and a reproducible bootstrap. Keep generated files distinguishable from hand-written code. |

Prefer caller-provided storage or an explicit allocator/arena argument. A
whole-file heap allocation is an explicit, documented exception, not a hidden
allocation: name and document ownership. Introduce iteration helpers only
when repeated loops show a real pattern. Keep public APIs small and test their
invariants before adding conveniences.

Study foreign ideas rather than translating syntax: compare an arena in RAD's
base layer with a documented allocator interface in Odin; ask what lifetime,
alignment and failure assumptions each makes. Read one source file and its
license first. Write your own small experiment from notes and compare outcomes;
don't copy large implementations or assume every technique is portable C.

## Make the reader useful for research

The project's own research problem is a feature source. Begin with local,
offline material; reports should contain **evidence with locations**, not just
counts. Do not require web access or an external indexer to build or run.

| When | Small capability / example | Test and honest boundary |
| --- | --- | --- |
| Phase 3 | `scan src` lists files, bytes and lines; later optionally list local `.md`/`.txt` documents too. | Check a tiny mixed tree; default C scan behavior stays clear. A document list is not full-text search. |
| Phases 4–5 | A literal text search can return `path:line: excerpt`; index definitions and includes with source locations. | Test repeated matches and long lines. Text matches are not semantic references. |
| Phase 6 | Search a symbol and show possible callers/callees with paths and lines. | Mark edges **heuristic/unresolved**; macros, function pointers and conditional compilation can mislead. |
| Phases 7–8 | Show bounded ASCII neighborhoods and findings with evidence, limitations and severity. | Unknown is not clean: missing-include and unused-static results are candidates, not authoritative diagnostics. |
| Phases 9–10 | For a selected local codebase, save a small report with root, tool/schema version, filters and timestamp or revision if known. | Compare a few results manually and optionally with ctags/cppcheck; neither is ground truth. Preserve notes when the index changes. |
| Later | Persistent local notes, bookmarks, cross-references and incremental refresh if repeated manual work justifies them. | Start with plain text; stale data and renamed files must be visible, never silently trusted. |

Example research loop: ask “where is memory allocated?”, search `alloc`, open
one result in an ordinary editor, write five notes including path and line,
then add a tiny fixture for a pattern the tool missed. For documentation,
start with local Markdown/plain text; PDFs, remote fetching, semantic search,
and automatic summaries are optional future work, not prerequisites. Keep
notes separate from generated reports so rebuilding an index cannot erase them.

C tokenization and parsing are hard: directives, escaped newlines, comments,
conditional branches, compiler extensions and macros alter what code means.
Year one can report lexical evidence without implementing full preprocessing or
type resolution. An Odin repository includes Odin, not just C: scan its files
and use literal search where appropriate; label C-tokenizer results on `.odin`
files unsupported rather than claiming Odin parsing. Study GCC in small local
subtrees; never infer project-wide completeness from a subset.

## A realistic year and optional side quests

The phase estimates add up to **44 weeks**; a separate buffer week after each
of the first 11 phases makes roughly **55 weeks**, before unexpected gaps. Use
44 weeks as an aspirational sequence, not a deadline: share about 8 buffer weeks
across the year where possible and carry unfinished work forward. Skipping a
week is allowed; the plan slides. Each phase's definition of done remains the
normal gate. If reality requires a smaller scope, explicitly record a revised
checkpoint and parked items rather than silently skipping them.

| Existing phases | Minimum useful checkpoint | Research or side quest, only after the checkpoint |
| --- | --- | --- |
| 1–2 | Safe file lifecycle and tested small base abstractions. | Compare one ownership idea from RAD or Odin; keep notes, no API rewrite. |
| 3–4 | Offline scan and a tokenizer on small fixtures; dogfood your own code. | Literal local document search; learn lexical edge cases. |
| 5–6 | Useful, labeled function/include/macro/call index with locations; candidate v0.1.0. | Compare a few results with ctags if installed; it is optional. |
| 7–8 | Readable bounded views and a few low-noise, labeled diagnostics. | Compare candidate diagnostics with cppcheck if installed; keep disagreements. |
| 9–10 | One deterministic generator and real-codebase studies on manageable subsets. | Investigate preprocessing in the GCC manual; prototype an optional restricted experiment. |
| 11–12 | Measure on your hardware, repair observed bottlenecks, document limits and decide release readiness. | Plan the next tool; defer features rather than forcing a 1.0 tag. |

The existing stabilization phase asks for a 1.0 tag. Treat that as a **decision
gate**, not a mandatory deadline: release 1.0 only if the documented CLI and
schema stability promise can be kept; otherwise publish an appropriate 0.x
release and explain the remaining work. A speed improvement or zero warnings on
one compiler is not proof of correctness; measure the same workload before and
after, and keep a justified change even if it does not speed things up.

A side quest is one bounded learning experiment after the current checkpoint,
not a new phase dependency. Record a question, time limit, expected output and
whether to keep or park it. Return to the last session log afterwards. Keep
existing backlog ideas, including full preprocessing, GUI, plugins and language
support; later placement is not rejection.

## A coding environment, built out of small tools

A future environment need not start as an IDE. Aim first for independent CLI
commands with plain text/JSON, exit codes and source locations, usable from any
editor. One possible progression, each conditional on a real use case:

1. **Research workbench:** local scan, search, index, reports and notes; open
   a result by its `path:line` in whatever editor is already available.
2. **Build assistant:** a tiny project manifest or command runner that displays
   commands and results; keep plain gcc builds possible and offline.
3. **Generators:** enum-to-string or struct-dump output from a deliberately
   constrained schema. Write the generator in C, build it with gcc first, run
   it to create output, then compile the program using that output. Decide
   whether generated output is checked in; test that regeneration is identical.
4. **Transpiler experiment:** define a tiny non-C input language, e.g. `say 2`,
   emit a small C program, and compile that output separately with gcc. Test
   invalid input and locations. This is not full C or Odin translation.
5. **Tiny compiler experiment:** choose a tiny expression language; first emit
   an inspectable stack instruction listing or C, then, if desired, explore a
   real target. A compiler for a small language is feasible; a general C
   compiler is a much larger undertaking.
6. **Debugger experiment:** start by navigating logs or inspecting symbols on
   a small program. A real interactive debugger needs OS-specific process
   control, debug information and substantial platform work; keep it a later
   optional project, not a promised year-one deliverable.

These may share source locations, errors and buffers from the base layer, but
should not require a giant framework. Windows and Termux platform differences
remain explicit. The browser/GUI idea can return later as a frontend to the
same reports; no old idea needs to be discarded.

## Practical guardrails for later phases

- **CLI safety:** validate `argc` before accessing `argv[2]`; the Phase 3
  skeleton's `argc >= 2` is insufficient for `scan <path>`. Check missing and
  extra arguments, return nonzero on failure, send diagnostics to stderr.
- **Walking:** C11 does not define directory enumeration. Isolate POSIX
  `dirent` and Windows APIs. Define symlink/reparse-point handling, depth and
  path limits, unreadable directories and deterministic ordering; avoid loops.
  `FindFirstFileA` has Windows encoding limitations; document them until a
  deliberate Unicode solution is tested. Never silently truncate names.
- **Reports:** escape JSON strings correctly, including control characters;
  choose and document encoding and behavior for invalid bytes. Write a schema
  field, stable ordering and fixtures. On a schema change bump the field as
  the release guide says; do not imply a heuristic index is complete.
- **Tests:** exercise the actual executable as well as small unit programs:
  empty files, missing paths, final newline, paths with spaces, large input,
  malformed tokens and failure cleanup. On each available platform run debug
  and release builds; explicitly note when the other platform was not tested.
- **Errors:** attach input path/location separately from implementation
  `__FILE__:__LINE__`, with documented lifetime; preserve a real `errno`
  immediately when relevant. Do not invent an OS cause when none is supplied.
- **PRs:** follow [workflow-git.md](../guides/workflow-git.md): first stable
  rewrite PR, then small reviewable features. Preserve old browser history by
  verifying a tag or archive branch points at the actual old commit before
  removing anything. Experiments stay separate; do not merge half-finished work.

## How to study and verify ideas

Use [reading-list.md](../guides/reading-list.md) and
[studying-codebases.md](../guides/studying-codebases.md) as the starting list.
For each weekly reading session choose a **specific local file** and a single
question; if a named path differs in your checkout, find it first and note the
actual path/revision. Example notes: purpose, data flow, ownership, failure,
one idea/question. Make a tiny independent C version in a later hand-written
session and compare behavior and constraints, not merely appearance. Keep notes
short and cite source paths/versions; check licensing before copying code.

Useful directions to verify in your local copies or official documentation:
RAD Debugger's base layer (memory and data organization), Odin's allocator and
core-library conventions (an idea source, not C syntax), GCC's preprocessor
manual and a small source subtree, and C library documentation for each API you
use. Read official language documentation when investigating other systems
languages; defer deep compiler internals until the corresponding experiment.
Optional comparison tools are learning aids, not build dependencies. Save the
materials you need locally ahead of time when connectivity is available.

## Assumptions and next decision

- Windows gcc means a working Windows gcc distribution is already installed;
  Termux has gcc available. Neither is verified here. Plain gcc is enough for
  building the project; OS directory APIs still need platform-specific code.
- The existing plan remains the default. The added research features and
  tool-building experiments are proposals, not promises or new gates.
- Next session: choose the Phase 1 baseline or lifecycle contract above, then
  log what happened. Keep new ideas in [BACKLOG.md](../ideas/BACKLOG.md).
