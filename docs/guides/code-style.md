# Code style for perspecta_see

Naming, headers, includes, const correctness, and ownership. Pick one style
and stay consistent — consistency matters more than which style you picked.

## Naming

| Thing | Style | Example |
| ----- | ----- | ------- |
| Types | `PascalCase` | `Error`, `File`, `String`, `Arena`, `Token` |
| Functions | `PascalCase` | `File_open`, `error_print`, `string_equals` |
| Variables / fields | `snake_case` | `file_path`, `out_bytes`, `is_open` |
| Macros | `SCREAMING_SNAKE` | `ERROR`, `ERROR_OK`, `STRING_LITERAL` |
| Error codes | `ERR_` prefix | `ERR_FILE_OPEN`, `ERR_INVALID_ARGUMENT` |
| Header guards | `PERSPECTA_<FILE>_H` | `PERSPECTA_FILE_H` |

Functions and types share `PascalCase` (matching the existing `File_open`,
`Error`, `ErrorCode`). This is deliberate: read `File_open` as "the `open`
operation on the `File` type".

Rationale for one style everywhere: you are self-taught and work in short
sessions. Switching conventions costs more than the convention itself.

## Header guards

Every project header:

```c
#ifndef PERSPECTA_ERROR_H
#define PERSPECTA_ERROR_H
...
#endif
```

Guard name: `PERSPECTA_` + file name in uppercase + `_H`. Never skip the guard.

## Include rules

- Project headers: quotes — `#include "error.h"`, `#include "file.h"`.
- System headers: angle brackets — `#include <stdio.h>`, `#include <errno.h>`.
- Order in a `.c` file: its own header first (so the header is self-contained),
  then project headers, then system headers.
- Never include a `.c` file.

Example:

```c
#include "file.h"
#include "error.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
```

## Const correctness

- Input pointers that are not modified: `const`.
```c
bool string_equals(String a, String b);                 /* by value, fine */
Error File_open(File *self, const char *path, const char *mode);
```
- Output through non-const pointers:
```c
Error File_read_whole(File *self, u8 **out_bytes, usize *out_size);
```
- `String.data` is `const char *` — strings borrowed from file buffers are not
  owned by the `String`.

## Ownership rules

1. **Who allocates, who frees.** If a function allocates (malloc/arena_alloc),
   its name says so (`create`, `new`, `arena_alloc`) and the doc comment says
   who frees it.
2. **Arena owns its buffer.** Caller frees the arena; never individual arena
   allocations.
3. **`File` owns its handle.** `File_open` takes the handle; `File_close`
   releases it. Caller must call `File_close` exactly once per successful open.
4. **No hidden allocation.** No function allocates behind the caller's back.
   Functions that need memory take a caller-provided buffer or an `Arena *`.
   Anything that mallocs is named `create`/`new` and documented.

Example comment at the top of `arena.h`:

```c
/* Arena owns its buffer. Caller frees the arena, never individual allocations. */
```

## Error handling

Follow `docs/guides/error-handling.md`: return `Error`, deliver results through
pointers, only `main()` prints, capture `errno` immediately.

## What to avoid

- `strcpy`/`strcat` without a checked length (use bounded copies).
- Magic numbers (`-1`, `1` as error signals) — use `ErrorCode`.
- Hidden mallocs in "getter"-style functions.
- Global error variables.
- `assert()` for runtime failures (missing file, bad input).
