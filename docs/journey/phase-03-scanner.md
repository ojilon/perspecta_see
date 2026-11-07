# Phase 3: Scanner (~4 weeks)

Walk a directory, list files by extension, compute line and size stats, output
txt then json. First useful tool: a project summary. Dogfooding starts here —
run it on your own repos from day one of this phase.

## Goal
A `perspecta scan <path>` command that walks a directory tree, groups files by
extension, counts lines and bytes, and writes a readable txt report followed by
a json report.

## Why it matters
This is the first command that is actually useful on real code. It forces the
file API, string type, and array to work together, and it produces output you
can eyeball — the smallest possible end-to-end tool.

## Prerequisites
- Phase 2 definition of done.
- Note: directory walking is OS-specific (`dirent.h` on Termux/POSIX,
  `FindFirstFile` on Windows). Keep the two implementations behind one
  `perspecta_scan_dir` function so the rest of the code stays portable.

## Sessions

### Session 1 — scan command skeleton ([hand-written])
**Goal.** `main.c` recognizes `perspecta scan <path>` and prints the path.
**Shape shown first.**
```c
if (argc >= 2 && string_equals(argv[1], STRING_LITERAL("scan"))) {
    printf("scan: %s\n", argv[2]);
    return 0;
}
```
**Expected result.** `perspecta.exe scan src` prints `scan: src`.
**Stop here.** Command parses; no walking yet.

### Session 2 — directory open/read signature ([hand-written])
**Goal.** One function that lists entries of a directory.
**Shape shown first.**
```c
typedef struct {
    char name[260];
    bool is_directory;
} DirEntry;

Error perspecta_dir_open(const char *path, void **out_handle);
Error perspecta_dir_read(void *handle, DirEntry *out);
void perspecta_dir_close(void *handle);
```
**Expected result.** Header compiles.
**Stop here.** Declarations compile. Platform bodies come next.

### Session 3 — directory body: Windows ([hand-written])
**Goal.** Implement with `FindFirstFileA`/`FindNextFileA`.
**Expected result.** Compiles on Windows with `-D_CRT_SECURE_NO_WARNINGS`.
**Stop here.** `perspecta_dir_read` fills `DirEntry`.

### Session 4 — directory body: Termux ([hand-written])
**Goal.** Implement with `dirent.h`.
**Expected result.** Compiles on Termux/Android.
**Stop here.** Same `DirEntry` shape as Windows.

### Session 5 — visible result: list `src/` ([hand-written])
**Goal.** Print every entry name in `src\`.
**Shape shown first.** Loop `perspecta_dir_read`, print `name` and `is_directory`.
**Expected result.** Screen shows `error.c`, `error.h`, `file.c`, `file.h`, `main.c`.
**Stop here.** Directory listing works.

### Session 6 — recursive walk + extension filter ([hand-written])
**Goal.** Walk subdirectories; keep only files ending in `.c` or `.h`.
**Shape shown first.** If `is_directory`, recurse; else check last 2 chars with `string_equals` on a slice.
**Expected result.** Program prints only `.c`/`.h` paths under the given root.
**Stop here.** `perspecta scan src` lists only source files.

### Session 7 — per-file stats ([hand-written])
**Goal.** For each source file: line count (Phase 1 logic) and byte count.
**Shape shown first.** Open with `File_open`, count `\n` with `fgetc`, track `size`, close.
**Expected result.** Prints `src\main.c 135 lines 4120 bytes`.
**Stop here.** Stats match `wc`-style counting on the same files.

### Session 8 — txt report ([hand-written])
**Goal.** Group by extension and print a summary table.
**Shape shown first.** Array of `{ext, file_count, total_lines, total_bytes}`; print aligned columns.
**Expected result.**
```
Extension  Files  Lines  Bytes
.c         3      412    12340
.h         2      89     2310
```
**Stop here.** Table prints for `perspecta scan .`

### Session 9 — reading session: your own repo ([reading])
**Goal.** Read one small file of a project you already own (not RAD Debugger yet).
**What to read.** Pick one file you wrote, 30 minutes, write 5 notes: what it does, how errors are handled, one thing you would change.
**Expected result.** 5 notes in session log.
**Stop here.** Notes written.

### Session 10 — json output ([hand-written])
**Goal.** Emit the same summary as json.
**Shape shown first.**
```c
printf("{\n");
printf("  \"files\": [\n");
/* one object per file: path, ext, lines, bytes */
printf("  ],\n");
printf("  \"summary\": { \"total_files\": %zu, \"total_lines\": %zu }\n");
printf("}\n");
```
**Expected result.** `perspecta scan . --format json` prints valid-looking json; save to `report.json` and eyeball it.
**Stop here.** Json file created and opens in a text editor.

### Session 11 — `--format` flag ([hand-written])
**Goal.** Choose txt or json from the command line.
**Shape shown first.** `if (string_equals(arg, STRING_LITERAL("--format")))` then next arg is `txt` or `json`.
**Expected result.** `perspecta scan . --format txt` and `--format json` both work.
**Stop here.** Flag parsing works; unknown format prints an error.

### Session 12 — output to file ([hand-written])
**Goal.** `--output report.txt` writes the report to a file instead of stdout.
**Shape shown first.** `File_open(&out, path, "wb")`, write report there, `File_close`.
**Expected result.** Running with `--output` creates a file you can open.
**Stop here.** Report written to disk.

### Session 13 — scan your own repos ([hand-written])
**Goal.** Dogfood: run `perspecta scan` on two of your own repositories.
**Expected result.** Two reports (txt or json) exist; note any crash or wrong count in the session log.
**Stop here.** Tool used on real code; problems noted.

### Session 14 — warnings cleanup ([hand-written])
**Stop here.** Zero warnings with `-Wall -Wextra -Wpedantic`.

### Session 15 — buffer / catch-up
**Stop here.** Sessions 1–14 complete.

### Session 16 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
`perspecta scan <path> [--format txt|json] [--output file]` produces a project
summary, tested on your own repos.

## Definition of done
- [ ] `perspecta scan` walks a directory recursively (Windows + Termux)
- [ ] Filters by extension (`.c`, `.h`)
- [ ] Line and byte stats per file
- [ ] Txt table report
- [ ] Json report
- [ ] `--format` and `--output` flags work
- [ ] Run on at least two of your own repos
- [ ] Clean build with warnings on

## What to read
- `docs/guides/config.md` — how CLI flags grow into config files (this phase uses flags only).
- gcc manual, warnings section (verify title/link) — for build flags.

## Buffer
One buffer week before Phase 4.
