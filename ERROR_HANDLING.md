# Error handling in perspecta_see

A guide to grow a small custom error system alongside the project.
Written to be followed in small steps (30 min to 1 hour each). Nothing here
has to be done at once. Each stage builds on the previous one, and each stage
ends with something that compiles and runs.

Build command used throughout (Windows, gcc):

```
gcc -std=c11 -Wall -Wextra -o perspecta.exe src/main.c src/error.c
```

(`-std=c11` is only needed from Stage 6 on, for anonymous unions. It is fine to
use it from the start.)

---

## The idea in one paragraph

Every function that can fail tells its caller **whether it failed and why**,
in the same way every time. The function that *detects* the problem does not
print or exit. It builds an `Error` and returns it. The caller decides what to
do: recover, retry, or pass it upward. Only `main()` (or the top-level layer)
prints errors and chooses the exit code.

Three separate jobs, kept separate:

| Job        | Who does it                                   |
| ---------- | --------------------------------------------- |
| Detect     | The function where the failure happens        |
| Report     | `main()` / top layer, using `error_print()`   |
| Recover    | The caller that knows what a sensible fallback is |

---

## Where we are today (Stage 0)

The first file has `FileInfo`, `Unpack_for_file_properties()` and `main()`.
Failure is handled only by `return 1` when no argument is given, and
`strncpy` silently cuts paths longer than 259 characters. That second one is
exactly the kind of silent failure this guide removes.

---

## Stage 1: the error module (Option A foundation)

Create two files. Keep them tiny. They will be reused by every later part of the
project.

```
perspecta_see/
├── src/
│   ├── error.h
│   ├── error.c
│   └── main.c
└── ERROR_HANDLING.md
```

### `src/error.h`

```c
#ifndef PERSPECTA_ERROR_H
#define PERSPECTA_ERROR_H

#include <stdbool.h>

typedef enum {
    ERR_NONE = 0,          /* must stay 0: zero-initialised Error means "no error" */

    ERR_INVALID_ARGUMENT,
    ERR_FILE_OPEN,
    ERR_FILE_READ,
    ERR_INTERNAL
} ErrorCode;

typedef struct {
    ErrorCode code;
    const char *message;   /* MUST be a string literal (see note below) */
    const char *file;      /* filled by the macro */
    int line;              /* filled by the macro */
} Error;

Error error_make(ErrorCode code, const char *message, const char *file, int line);
bool error_is_ok(const Error *err);
const char *error_name(ErrorCode code);
void error_print(const Error *err);

#define ERROR(code, message) error_make((code), (message), __FILE__, __LINE__)
#define ERROR_OK ((Error){0})

#endif
```

### `src/error.c`

```c
#include "error.h"
#include <stdio.h>

Error error_make(ErrorCode code, const char *message, const char *file, int line) {
    Error err;
    err.code = code;
    err.message = message;
    err.file = file;
    err.line = line;
    return err;
}

bool error_is_ok(const Error *err) {
    return err->code == ERR_NONE;
}

const char *error_name(ErrorCode code) {
    switch (code) {
        case ERR_NONE:             return "ERR_NONE";
        case ERR_INVALID_ARGUMENT: return "ERR_INVALID_ARGUMENT";
        case ERR_FILE_OPEN:        return "ERR_FILE_OPEN";
        case ERR_FILE_READ:        return "ERR_FILE_READ";
        case ERR_INTERNAL:         return "ERR_INTERNAL";
    }
    return "ERR_UNKNOWN";
}

void error_print(const Error *err) {
    fprintf(stderr, "error [%s]: %s (%s:%d)\n",
            error_name(err->code), err->message, err->file, err->line);
}
```

### Things to understand here

- **Why the message must be a string literal.** `Error` is returned *by value*
  and outlives the function that made it. A string literal lives for the whole
  program. A local `char buf[100]` dies when the function returns, and the
  `message` pointer would then point at garbage. Rule: `ERROR(code, "literal text")`
  only. If you ever need formatted messages, that is a later, deliberate change.
- **Why `ERR_NONE = 0`.** `Error err = {0};` and `ERROR_OK` both mean success
  without any extra work.
- **Why no memory allocation.** Reporting an error should never itself fail
  because of an allocation.
- **Add codes only when a call site needs one.** Five codes is enough for now.
  Do not pre-build a list of 30.

> End of Stage 1 check: project compiles with `error.c` linked in. Nothing uses
> it yet. That is fine.

---

## Stage 2: use it in today's code

Convert `Unpack_for_file_properties` so it reports problems instead of
silently truncating. This is a surgical edit: same function name, same fields.

**Before**

```c
void Unpack_for_file_properties(FileInfo *self, const char *path) {
    strncpy(self->file_path, path, MAX_PATH_LENGTH-1);
    self->file_path[MAX_PATH_LENGTH-1] = '\0';
    self->line_count = 0;
    self->file_size = 0;
}
```

**After**

```c
Error Unpack_for_file_properties(FileInfo *self, const char *path) {
    if (self == NULL || path == NULL) {
        return ERROR(ERR_INVALID_ARGUMENT, "NULL argument");
    }
    if (strlen(path) >= MAX_PATH_LENGTH) {
        return ERROR(ERR_INVALID_ARGUMENT, "Path is too long");
    }

    strcpy(self->file_path, path);   /* safe: length checked above */
    self->line_count = 0;
    self->file_size = 0;

    return ERROR_OK;
}
```

Notes:

- The function now returns `Error`. This is the Option A shape: **return the
  error, put results in the struct or through pointers.**
- We check the length *first*, so `strcpy` is safe. A too-long path is now a
  reported failure, not a silent cut.
- On the error path nothing in `self` was touched.

**`main()` changes** (add `#include "error.h"` at the top):

```c
int main(int argc, char *argv[]) {
    FileInfo Target_file;

    if (argc < 2) {
        fprintf(stderr, "Usage: perspecta.exe <path_to_file.c>\n");
        return 1;
    }

    Error err = Unpack_for_file_properties(&Target_file, argv[1]);
    if (!error_is_ok(&err)) {
        error_print(&err);
        return 1;
    }

    printf("Path to file: %s\n", Target_file.file_path);
    printf("Line count: %d\n", Target_file.line_count);

    return 0;
}
```

Test it:

```
perspecta.exe                      -> usage message, exit 1
perspecta.exe src\main.c           -> prints path, line count 0
perspecta.exe <300-char path>      -> error [ERR_INVALID_ARGUMENT]: Path is too long
```

(Usage text goes to `stderr` and ends with `\n`. Small habit, easy to adopt now.)

---

## Stage 3: opening a file and counting lines

First real source of errors. New function with the same Option A shape:
**returns `Error`, writes results into `FileInfo`.**

```c
Error fileinfo_scan(FileInfo *self) {
    Error err = ERROR_OK;
    FILE *file = NULL;

    int ch;
    int last = '\n';     /* so an empty file counts as 0 lines */
    int lines = 0;
    long size = 0;

    if (self == NULL) {
        return ERROR(ERR_INVALID_ARGUMENT, "NULL FileInfo");
    }

    file = fopen(self->file_path, "rb");
    if (file == NULL) {
        err = ERROR(ERR_FILE_OPEN, "Could not open file");
        goto cleanup;
    }

    while ((ch = fgetc(file)) != EOF) {
        size++;
        if (ch == '\n') lines++;
        last = ch;
    }

    if (ferror(file)) {
        err = ERROR(ERR_FILE_READ, "Error while reading file");
        goto cleanup;
    }

    if (last != '\n') lines++;   /* last line had no trailing newline */

    /* only write results once everything succeeded */
    self->line_count = lines;
    self->file_size = size;

cleanup:
    if (file != NULL) fclose(file);
    return err;
}
```

Use it in `main()` right after the unpack call, with the same pattern:

```c
err = fileinfo_scan(&Target_file);
if (!error_is_ok(&err)) {
    error_print(&err);
    return 1;
}
```

What to learn from this stage:

- **`goto cleanup`.** One exit path that releases whatever was acquired.
  `file` starts as `NULL`, so cleanup can safely ask "did we open it?"
- **Results are written last.** If the scan fails halfway, `FileInfo` is not
  left with a half-counted `line_count`.
- **`fgetc` returns `int`, not `char`.** Needed so `EOF` can be told apart
  from a real byte.
- **`ferror` after the loop.** The loop ends on `EOF` for both "finished" and
  "read failed". `ferror` tells you which.

Test it: a normal file, an empty file, a file without a final newline, and a
path that does not exist (`ERR_FILE_OPEN`).

> Compare counts with `find /c /v "" file.c` on Windows to check yourself.

---

## Stage 4: keep the operating system's reason (`errno`)

`ERR_FILE_OPEN` says *what* failed. The OS knows *why* (not found, permission
denied, ...). Keep it.

**Surgical edits:**

1. `error.h`: add one field to `Error`:

```c
typedef struct {
    ErrorCode code;
    int system_code;       /* errno value, 0 if not applicable */
    const char *message;
    const char *file;
    int line;
} Error;
```

2. `error.c`: in `error_make`, add `err.system_code = 0;`

3. `error.c`: in `error_print`, show it when set (add `#include <string.h>`):

```c
void error_print(const Error *err) {
    fprintf(stderr, "error [%s]: %s (%s:%d)\n",
            error_name(err->code), err->message, err->file, err->line);
    if (err->system_code != 0) {
        fprintf(stderr, "  cause: %s\n", strerror(err->system_code));
    }
}
```

4. In `fileinfo_scan` (add `#include <errno.h>`), capture `errno` **immediately**
   after the failing call, before anything else can change it:

```c
file = fopen(self->file_path, "rb");
if (file == NULL) {
    int saved = errno;
    err = ERROR(ERR_FILE_OPEN, "Could not open file");
    err.system_code = saved;
    goto cleanup;
}
```

Now a missing file prints:

```
error [ERR_FILE_OPEN]: Could not open file (src/main.c:52)
  cause: No such file or directory
```

---

## Stage 5: recovery vs propagation

Not every error should travel up unchanged. The caller sometimes knows a
fallback. Two examples for later parts of the project.

**Propagate**: the caller cannot do anything useful, so it passes the error up:

```c
Error load_target(FileInfo *info, const char *path) {
    Error err = Unpack_for_file_properties(info, path);
    if (!error_is_ok(&err)) return err;

    err = fileinfo_scan(info);
    if (!error_is_ok(&err)) return err;

    return ERROR_OK;
}
```

**Recover**: the caller inspects the code and handles one specific case. For
example, an optional index file that may not exist yet:

```c
Error err = load_index("perspecta.idx");
if (err.code == ERR_FILE_OPEN) {
    /* first run: no index yet, start empty */
    err = ERROR_OK;
}
if (!error_is_ok(&err)) {
    error_print(&err);
    return 1;
}
```

Rule: recover only from the exact codes you understand. Everything else is
propagated.

---

## Stage 6: Option B, a `Result` that carries a value OR an error

**When to introduce it.** Option A (return `Error`, result via pointer) is
enough as long as functions have one obvious output. Reach for Option B when:

- the function's *main purpose* is to return a value (a size, a count, a pointer), and out-parameters start to feel clumsy;
- you want to chain calls like `get_size(path)` inside other expressions;
- you keep forgetting which pointer argument is the output.

Do not rewrite working Option A functions. Use B for new functions where it fits.
Both styles can live in the same project.

### Define result types (put in `error.h`)

C has no generics, so define one per value type you really use:

```c
typedef struct {
    bool ok;
    union {
        long value;
        Error error;
    };                      /* anonymous union: needs C11 (-std=c11) */
} LongResult;

#define LONG_OK(v)   ((LongResult){ .ok = true,  .value = (v) })
#define LONG_ERR(e)  ((LongResult){ .ok = false, .error = (e) })
```

The union holds **either** the value **or** the error, never both.
Only read `.value` when `ok` is true, and only read `.error` when it is false.

### Write a function with it

```c
LongResult file_size_of(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        int saved = errno;
        Error e = ERROR(ERR_FILE_OPEN, "Could not open file");
        e.system_code = saved;
        return LONG_ERR(e);
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return LONG_ERR(ERROR(ERR_FILE_READ, "Could not seek to end"));
    }

    long size = ftell(file);
    fclose(file);

    if (size < 0) {
        return LONG_ERR(ERROR(ERR_FILE_READ, "Could not get file size"));
    }
    return LONG_OK(size);
}
```

### Use it

```c
LongResult r = file_size_of(Target_file.file_path);
if (!r.ok) {
    error_print(&r.error);
    return 1;
}
Target_file.file_size = r.value;
```

Compare with Option A for the same job:

```c
Error file_size_of(const char *path, long *out_size);   /* A */
LongResult file_size_of(const char *path);              /* B */
```

Both are valid. B makes it impossible to read the value without first
looking at `ok`, which is the main benefit.

Later you may want other result types (`StringResult` with `char *value`, a
`PtrResult` with `void *value`). Add them when a real function needs one.

### Bonus for later: a small propagate macro

Only once the repeated `if (!error_is_ok(...)) return err;` lines start to
annoy you:

```c
#define TRY(expr) do {                      \
    Error try_err_ = (expr);                \
    if (!error_is_ok(&try_err_)) return try_err_; \
} while (0)
```

Usage: `TRY(Unpack_for_file_properties(&info, path));`

Warning: `TRY` returns immediately. **Do not use it in a function that has
a `cleanup:` label to run.** There, keep the explicit `goto cleanup`.

---

## Project rules (copy into the README when ready)

1. Named error codes, never bare `-1` / `1` / magic numbers.
2. Every function that can fail returns `Error` (Option A) or a `Result` (Option B).
3. Functions do not print errors or call `exit()`. Only `main()` reports.
4. The caller decides: recover, retry, or propagate.
5. Every acquired resource (file, memory) has one cleanup path (`goto cleanup`).
6. Never ignore a returned error without a written reason.
7. Error messages are string literals. No allocation in the error path.
8. Capture `errno` immediately after the failing call.
9. No global error variable.
10. `assert()` is for bugs in the program, not for runtime failures like a missing file.
11. Add a new `ErrorCode` only when a call site needs it.

---

## Session roadmap (each item is one small session)

- [ ] Stage 1: `error.h` / `error.c`, compile
- [ ] Stage 2: convert `Unpack_for_file_properties`, update `main()`
- [ ] Stage 3: `fileinfo_scan`: open, count lines and bytes, `goto cleanup`
- [ ] Stage 4: `system_code` / `errno`
- [ ] Stage 5: write one recover case (e.g. optional index file)
- [ ] Stage 6: `LongResult` + `file_size_of`
- [ ] Add `tests/error_tests.c`: small checks that `ERROR_OK` is ok, `ERROR(...)` sets code/line, `error_name` covers every code
- [ ] Later: add codes as features need them (`ERR_PARSE`, `ERR_NOT_FOUND` for search/indexing, `ERR_OUT_OF_MEMORY` when allocation starts)
- [ ] Later, optional: move `error.h` / `error.c` into a shared folder so other C projects can reuse them

## Reading, when you want more

- SEI CERT C: error-handling recommendations (ERR rules)
- "Patterns for Returning Error Information in C" (Preschern, 2019 paper)
