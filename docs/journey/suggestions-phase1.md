# Phase 1 suggestions (aligned with the practical addendum)

This is a **companion to** [phase-01-foundation-i.md](phase-01-foundation-i.md)
and [practical-addendum.md](practical-addendum.md). It does not replace either.
Where the original Phase 1 sessions assume a different starting point, prefer
the order and safety notes below. Hand-write the core; use AI only for review
and test inputs.

## Current baseline (verified on this branch)

- `Error` / `ERROR` / `error_print` work.
- `File_open` / `File_close` exist and are used by `Count_lines`.
- `main.c` prints path, size and line count; it already goes through the `File`
  API (no raw `fopen` in the hot path).
- No `types.h`, no build script, no `File_read_chunk` / `File_read_whole`, no
  tests directory, no formal lifecycle documentation.

The original Phase 1 Session 12 ("main.c onto the File API") is already done.
Treat it as complete and skip it.

## Recommended session order (safer path)

Each item is one 15–30 minute sitting. End compiling, with a visible check and a
one- or two-line session log.

| # | Focus | Why this order |
| - | ----- | -------------- |
| 1 | **Baseline record** | Run `perspecta src/main.c` and a missing-file case. Write command + observed output. Establishes the ground truth the addendum asks for. |
| 2 | **Lifecycle contract (doc only)** | Document: `File f = {0}` is closed; `File_open` requires closed; path is borrowed; who closes. No code change yet. |
| 3 | **Lifecycle checks (tiny test)** | Open real file, close twice, open missing, attempt open-on-open. Confirm no leak / no destroyed handle. Prefer a small test main over changing production code first. |
| 4 | **`types.h`** | Fixed-width aliases (`i8`…`usize`). Header-only; do not force-use everywhere yet. Conditional on platform `stdint.h`. |
| 5 | **Build entry points** | Tiny `build.sh` + `build.bat` (or documented direct gcc). One successful build on the machine you have; note the other as unverified. |
| 6 | **`File_read_chunk` signature + body** | Caller-provided buffer. Spec: valid args, zero capacity, EOF = success with 0 bytes, outputs on failure. Prefer this before whole-file. |
| 7 | **Use chunks for stats** | Replace the direct `fgetc` loop in `Count_lines` (or a parallel path) with a chunk loop. Fixtures: empty, `a\n`, `a\nb`. Publish results only after successful read + close. |
| 8 | **`File_read_whole` only when needed** | Explicit ownership, NUL policy, empty file, overflow, partial read. Prefer a checked chunk loop for non-seekable input; if using seek/tell, restrict to seekable and check every result. |
| 9 | **One test program** | e.g. `tests/file_chunk_test.c` or `tests/file_lifecycle_test.c`. PASS/FAIL. Exercise the real executable too when feasible. |
| 10 | **Warnings cleanup** | `-Wall -Wextra -Wpedantic`; temporarily `-Werror` to find issues, then drop it from the daily script. |
| 11 | **Reading session** | One local file + five notes (contract, ownership, failure, surprise, question). Existing build still works. |
| 12 | **Phase checkpoint** | Definition of done below. Log what was parked. |

Skip or split any step that feels large. An unused declaration is better than an
unsafe body.

## Definition of done (refined)

- [ ] Lifecycle invariants for `File` are written down and checked by a tiny test
- [ ] `src/types.h` with the fixed-width aliases
- [ ] `build.sh` / `build.bat` (or equivalent documented commands) on at least one platform
- [ ] `File_read_chunk` implemented, with at least one fixture
- [ ] Line/size counting uses a safe read path (chunk preferred); close errors handled
- [ ] `File_read_whole` present only if needed, with ownership and failure tests
- [ ] At least one program under `tests/` prints PASS
- [ ] Clean build with `-Wall -Wextra -Wpedantic`
- [ ] Session logs present; original Session 12 marked done/skipped

## Style and naming reconciliation

The style guide shows both `File_open` and `error_print`. Prefer:

- `Type_operation` for type-associated operations (`File_open`, `File_close`)
- `lower_snake_case` for free functions (`error_print`, `error_is_ok`)

Do **not** mass-rename. Update the guide when convenient so examples match the
code that already exists.

## Guardrails carried from the addendum

- `File_open` on an already-open object currently resets and loses the old handle.
  Document or reject that case before adding more callers.
- `path` is borrowed; its lifetime must cover use.
- `ERROR`'s `__FILE__:__LINE__` is the *implementation* site, not the input path.
  Attach input location separately when you need it.
- Capture `errno` immediately; do not invent a system cause when none exists.
- Fixed 260-byte path buffer and `int`/`long` counters are real limits — handle
  overflow explicitly when touching those paths; do not expand scope in Phase 1.
- Whole-file sketches that do unchecked seek/alloc/read are incomplete shapes.
  Treat them as sketches, not copy-paste targets.

## What not to do in Phase 1

- No directory walking, tokenizer, or JSON yet.
- No mass rename or API redesign for aesthetics.
- No dependency on network, CMake-as-requirement, or an external test framework.
- Do not treat a 1.0 tag or full preprocessing as a Phase 1 concern.

## Next decision after these sessions

When the refined definition of done is met, move to Phase 2 (arenas, string
views, vectors) only for data that has already shown a real need. Keep new ideas
in [BACKLOG.md](../ideas/BACKLOG.md).
