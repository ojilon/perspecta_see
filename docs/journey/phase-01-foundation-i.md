# Phase 1: Foundation I (~4 weeks)

Finish the error and file APIs (open, close, read whole, read chunks), add
integer typedefs, a tiny build script, and small test programs. By the end,
`main.c` uses the `File` API instead of raw `fopen`.

## Goal
A small, reliable base layer: open a file through a handle, read it whole or in
chunks, report errors with `errno` reasons, and build everything with one
command.

## Why it matters
Every later phase (scanner, tokenizer, indexer) opens and reads files. If this
layer is shaky, every later phase inherits the bug. Error handling here sets the
convention for the whole project: return `Error`, deliver results through
pointers.

## Prerequisites
- Phase 0 (current code): `error.h/error.c`, `file.h/file.c` (open/close),
  `main.c` (FileInfo, prints path/size/lines) compile and run.

## Sessions

### Session 1 — integer typedefs ([hand-written])
**Goal.** One header with fixed-width integer aliases used everywhere.
**Shape shown first.**
```c
#ifndef PERSPECTA_TYPES_H
#define PERSPECTA_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef int8_t   i8;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
typedef size_t   usize;
typedef float    f32;
typedef double   f64;

#endif
```
**Expected result.** `src/types.h` exists; `gcc -std=c11 -Wall -Wextra -c src/types.h` compiles.
**Stop here.** Header compiles on its own. Do not use the types anywhere yet.

### Session 2 — build script ([hand-written])
**Goal.** One command builds the whole project, on Windows and Termux.
**Shape shown first.**
```bat
:: build.bat
gcc -std=c11 -Wall -Wextra -Wpedantic -o perspecta.exe src\main.c src\error.c src\file.c
```
```sh
# build.sh
gcc -std=c11 -Wall -Wextra -Wpedantic -o perspecta src/main.c src/error.c src/file.c
```
**Expected result.** `build.bat` (or `build.sh`) runs and `perspecta.exe`/`perspecta` appears.
**Stop here.** Double-click or run the script; the binary starts (usage message if no args).

### Session 3 — `File_read_whole` signature ([hand-written])
**Goal.** Declare whole-file reading in `file.h`.
**Shape shown first.**
```c
Error File_read_whole(File *self, u8 **out_bytes, usize *out_size);
```
**Expected result.** `file.h` gains the declaration; project still compiles (unused declaration is fine).
**Stop here.** Header compiles. Do not write the body yet.

### Session 4 — `File_read_whole` body ([hand-written])
**Goal.** Read an entire file into a heap buffer through the `File` handle.
**Shape shown first.**
```c
fseek(self->handle, 0, SEEK_END);
long size = ftell(self->handle);
fseek(self->handle, 0, SEEK_SET);
*out_bytes = malloc((usize)size);
fread(*out_bytes, 1, (usize)size, self->handle);
*out_size = (usize)size;
```
**Expected result.** `build.bat` compiles; no function calls it yet.
**Stop here.** Compiles clean. Allocation failure handling comes in session 6.

### Session 5 — visible result: dump a file's bytes ([hand-written])
**Goal.** Call `File_read_whole` from `main.c` and print bytes read.
**Shape shown first.**
```c
u8 *bytes = NULL;
usize size = 0;
Error err = File_read_whole(&file, &bytes, &size);
if (!error_is_ok(&err)) { error_print(&err); return 1; }
printf("bytes read: %zu\n", size);
free(bytes);
```
**Expected result.** `perspecta.exe src\main.c` prints `Path: ...`, `Size: ...`, `bytes read: 1352` (or your file's size).
**Stop here.** Program runs on `src\main.c` and prints byte count; `free(bytes)` is called.

### Session 6 — allocation failure in read_whole ([hand-written])
**Goal.** `malloc` failure is an error, not a crash.
**Shape shown first.**
```c
if (*out_bytes == NULL) {
    return ERROR(ERR_INTERNAL, "out of memory");
}
```
**Expected result.** Code path exists; compile only (hard to trigger on purpose).
**Stop here.** `NULL` check after `malloc`, returns `ERR_INTERNAL`.

### Session 7 — `File_read_chunk` signature ([hand-written])
**Goal.** Read a file in fixed-size chunks.
**Shape shown first.**
```c
Error File_read_chunk(File *self, u8 *buffer, usize size, usize *out_read);
```
**Expected result.** Declaration added to `file.h`; compiles.
**Stop here.** Header compiles. Body next session.

### Session 8 — `File_read_chunk` body ([hand-written])
**Goal.** `fread` into caller's buffer, report how many bytes were read.
**Shape shown first.**
```c
*out_read = fread(buffer, 1, size, self->handle);
if (ferror(self->handle)) {
    int saved = errno;
    return ERROR(ERR_FILE_READ, "read failed");
}
```
**Expected result.** Compiles; unused until session 9.
**Stop here.** Compiles clean.

### Session 9 — visible result: chunked dump ([hand-written])
**Goal.** Read `src\main.c` in 16-byte chunks and print each chunk's size.
**Shape shown first.**
```c
u8 buf[16];
usize read = 0;
while (true) {
    Error err = File_read_chunk(&file, buf, sizeof(buf), &read);
    if (!error_is_ok(&err)) { ... }
    printf("chunk: %zu bytes\n", read);
    if (read < sizeof(buf)) break;
}
```
**Expected result.** Program prints a list of `chunk: 16 bytes` lines ending with a smaller final chunk.
**Stop here.** Chunked read of `src\main.c` works end to end.

### Session 10 — `File` handle lifecycle review ([hand-written])
**Goal.** Make sure every open is closed, even on read error.
**Shape shown first.** Review `File_open`, `File_close`, `Count_lines` in `main.c`.
**Expected result.** No leaked handles in current code paths.
**Stop here.** Code review notes written in session log; no code change required unless a leak is found.

### Session 11 — reading session: a real base layer ([reading])
**Goal.** See how an experienced C project lays out its base layer.
**What to read.** One file in the RAD Debugger repo's base layer (e.g. its memory/arena or string module — locate it first, mark "verify" if unsure). Spend 30 minutes reading, write 5 notes.
**Expected result.** 5 notes in your session log about naming, ownership, or error style you might copy.
**Stop here.** Notes written; no code written.

### Session 12 — `main.c` onto the `File` API ([hand-written])
**Goal.** Replace raw `fopen` in `Count_lines` with `File_open`/`File_close`/`fgetc` on the handle.
**Shape shown first.** Swap `FILE *file = fopen(...)` for `File fileobject; File_open(&fileobject, ...)` and use `fileobject.handle` with `fgetc`.
**Expected result.** `perspecta.exe src\main.c` prints identical path/size/line count, now through the `File` API.
**Stop here.** Output identical to before; raw `fopen` removed from `main.c`.

### Session 13 — first test program ([hand-written])
**Goal.** A small program that checks `File_read_whole` on a known file.
**Shape shown first.** `tests/file_read_test.c` with a `main` that opens `src\error.h`, reads whole, asserts size > 0, prints PASS/FAIL.
**Expected result.** `gcc ... tests/file_read_test.c src/file.c src/error.c` compiles and prints PASS.
**Stop here.** Test program runs and prints PASS.

### Session 14 — warnings-as-errors cleanup ([hand-written])
**Goal.** Build with `-Werror` and fix every warning.
**Shape shown first.** Add `-Werror` to `build.bat` temporarily, run, fix each warning.
**Expected result.** `build.bat` with `-Wall -Wextra -Wpedantic -Werror` compiles zero warnings.
**Stop here.** Clean build; remove `-Werror` from daily script (keep it for releases).

### Session 15 — buffer / catch-up
**Goal.** Finish anything unfinished from sessions 1–14.
**Stop here.** All of sessions 1–14 compile and run.

### Session 16 — phase checkpoint
**Goal.** Verify the definition of done before Phase 2.
**Stop here.** Checklist below is complete.

## Deliverable
`perspecta.exe` built by `build.bat`/`build.sh`; `main.c` prints path, size, line
count using `File_open`/`File_close`/`File_read_whole`; `tests/` holds one small
test program; integer typedefs in `src/types.h`.

## Definition of done
- [ ] `src/types.h` with i8/u8/u16/i32/u32/i64/u64/usize/f32/f64
- [ ] `build.bat` and `build.sh` compile everything with one command
- [ ] `File_read_whole` and `File_read_chunk` implemented and tested
- [ ] `main.c` uses the `File` API (no raw `fopen`)
- [ ] One test program in `tests/` passes
- [ ] Clean build with `-Wall -Wextra -Wpedantic`
- [ ] Session logs written for every session

## What to read
- `docs/guides/code-style.md` (naming and ownership rules) — before Phase 2.
- `docs/guides/error-handling.md` — Option A conventions used above.

## Buffer
One buffer week sits between Phase 1 and Phase 2. Use it for exams, power cuts,
or catching up. Skipping it is fine.
