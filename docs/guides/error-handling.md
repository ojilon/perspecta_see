# Error handling in perspecta_see

Philosophy and conventions for this repo. Option A is used now; Option B is a
later, deliberate upgrade. Both styles can live side by side.

## The idea in one paragraph

Every function that can fail tells its caller **whether it failed and why**, in
the same way every time. The function that *detects* the problem does not print
or exit. It builds an `Error` and returns it. The caller decides: recover,
retry, or pass it upward. Only `main()` (or the top layer) prints errors and
chooses the exit code.

Three separate jobs, kept separate:

| Job        | Who does it                                   |
| ---------- | --------------------------------------------- |
| Detect     | The function where the failure happens        |
| Report     | `main()` / top layer, using `error_print()`   |
| Recover    | The caller that knows a sensible fallback     |

## Option A — return `Error`, result through pointers (now)

This is the project's default shape:

```c
Error File_open(File *self, const char *path, const char *mode);
Error File_read_whole(File *self, u8 **out_bytes, usize *out_size);
```

- **Return the error.** Caller checks `error_is_ok(&err)`.
- **Deliver results through pointers** (`out_bytes`, `out_size`, or fields of a struct passed in).
- **Write results last.** Only fill the output after every check passes, so a
  failed call leaves nothing half-written.
- **One cleanup path.** Every acquired resource (file, memory) is released on a
  single `goto cleanup` label.
- **Caller decides.** Recover only from error codes you understand; propagate
  everything else.

Example:

```c
Error load_target(FileInfo *info, const char *path) {
    Error err = Unpack_for_file_properties(info, path);
    if (!error_is_ok(&err)) return err;          /* propagate */

    err = File_read_whole(&file, &bytes, &size);
    if (!error_is_ok(&err)) return err;          /* propagate */

    return ERROR_OK;
}
```

Recovery example (optional index file that may not exist):

```c
Error err = load_index("perspecta.idx");
if (err.code == ERR_FILE_OPEN) {
    err = ERROR_OK;   /* first run: no index yet, start empty */
}
if (!error_is_ok(&err)) {
    error_print(&err);
    return 1;
}
```

## Option B — `Result` carrying a value OR an error (later)

Reach for B when a function's main purpose is to return a value and out-
parameters feel clumsy, or when you want to use the result inside expressions.

C has no generics, so define one result type per value type you really use
(put in `error.h`):

```c
typedef struct {
    bool ok;
    union {
        long value;
        Error error;
    };
} LongResult;

#define LONG_OK(v)   ((LongResult){ .ok = true,  .value = (v) })
#define LONG_ERR(e)  ((LongResult){ .ok = false, .error = (e) })
```

Rule: read `.value` only when `ok` is true; read `.error` only when false.

Do not rewrite working Option A functions. Use B for new functions where it fits.

## Conventions (project rules)

1. Named error codes only — never bare `-1`, `1`, or magic numbers.
2. Every function that can fail returns `Error` (Option A) or a `Result` (Option B).
3. Functions do not print errors or call `exit()`. Only `main()` reports.
4. The caller decides: recover, retry, or propagate.
5. Every acquired resource has one cleanup path (`goto cleanup`).
6. Never ignore a returned error without a written reason (log it if you must ignore).
7. Error messages are string literals. No allocation in the error path.
8. Capture `errno` immediately after the failing call, before anything else can change it.
9. No global error variable.
10. `assert()` is for bugs in the program, not for runtime failures like a missing file.
11. Add a new `ErrorCode` only when a call site needs one.

## Error struct fields

```c
typedef struct {
    ErrorCode code;       /* what failed */
    const char *message;  /* human-readable, string literal */
    int system_code;      /* errno value, 0 if not applicable */
    const char *file;     /* filled by ERROR macro (__FILE__) */
    int line;             /* filled by ERROR macro (__LINE__) */
} Error;
```

`ERR_NONE = 0` so zero-initialized `Error` means success. `ERROR_OK` is
`((Error){0})`.

`ERROR(code, message)` fills `file` and `line` automatically. Use it everywhere;
do not call `error_make` directly except inside `error.c`.

## Where things live

- `src/error.h` — struct, macros, declarations.
- `src/error.c` — `error_make`, `error_is_ok`, `error_name`, `error_print`.
- `docs/guides/code-style.md` — naming and ownership rules that error code follows too.

## When to add codes

Add one only when a call site needs it: `ERR_PARSE` for the tokenizer, `ERR_NOT_FOUND`
for search, `ERR_OUT_OF_MEMORY` when allocation starts. Do not pre-build a list
of 30 codes.
