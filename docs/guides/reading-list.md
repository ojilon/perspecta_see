# Reading list — what to read and when

Tied to the phases. Official and primary sources only. Items marked **verify**
are things I have not confirmed exist under that exact title/link — search for
them instead of trusting the wording.

## Read now (before or during Phase 1)

- **Beej's Guide to C** — already in your repo. Read the sections on pointers,
  memory, and file I/O as you write `file.c`.
- **`docs/guides/error-handling.md`** and **`docs/guides/code-style.md`** — the
  conventions this repo follows.
- **The gcc manual, warnings section** — verify the exact page; search "gcc
  warning options -Wall -Wextra". Read only the warnings you will use.

## Read during Phase 2 (foundations)

- **Articles on arena allocation and data-oriented layering by experienced C
  programmers** — verify titles/authors. Search "arena allocator C" and
  "data-oriented design C layers". Read one article, write 5 notes, compare
  with your `arena.h`.
- **RAD Debugger base layer** — one file (verify exact directory; look for
  `base/` or `src/base/` in the repo). 30 minutes, 5 notes on naming, memory,
  errors.

## Read during Phase 3–4 (scanner, tokenizer)

- **The gcc manual, preprocessor section** — verify title. Search "gcc
  preprocessor #include". Read only enough to understand what `#include` lines
  look like; you are not writing a preprocessor.
- **cppreference.com** — C standard library reference (verify it is still the
  primary reference you use). Use for `fopen`, `fread`, `fseek`, `ftell`,
  `errno`, `strerror`.

## Read during Phase 5–6 (indexing)

- **universal-ctags documentation** — verify title/link. Search "universal-ctags
  json output". Used to compare your index against ground truth.
- **Odin source** — one file or directory (verify path under `core/` or
  `vendor/`). 30 minutes, 5 notes on style.

## Read during Phase 7–8 (views, diagnostics)

- **cppcheck documentation** — verify title/link. Search "cppcheck manual".
  Read what it checks; used as the comparison baseline for your `check`
  command.

## Read during Phase 10 (real codebases)

- **GCC manual: preprocessor and warnings** — verify titles. High-level only:
  how macros are expanded, how the preprocessor runs, how GCC searches
  functions. GCC itself is too big to read fully; read one directory and one
  source file.
- **RAD Debugger** — base layer first (one file), then one source file.
- **Odin source** — one more file if not already read.
- **Your local GCC copy** — one directory + one source file (verify path;
  look under `gcc/`).

## Read during Phase 11–12 (hardening, stabilization)

- Re-read your own session logs — they are your best documentation.
- Any guide in `docs/guides/` that feels stale; update it.

## How to read

- 30 minutes of reading + 5 notes per session (one reading session per week).
- Notes: what the code does, one naming/ownership idea to copy, one question.
- If a link or title is uncertain, write "verify" next to it and search rather
  than guess.

## Postpone for months

- Full C preprocessing, macro expansion engines, type resolution, language
  servers, GUI, web frontend. These are year-2 topics at the earliest.
