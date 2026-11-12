# File handle lifecycle contract

## `{0}` means closed 
A `File` value initialized as `File f = {0};` (or with all fields zeroed) is considered **closed**:

- `f.handle == NULL`
- `f.path == NULL`
- `f.is_open == false`

Which is the only valid starting state for a `File` object.
Callers musr initialize every `File` they declase beofre passing it to `File_open`.

## File_open requirements
`File_open(&f, path, mode)` requires `f` to be closed (`f is_open == false`).

- On success: `f.hadle` is set. `f.path` stores the borrwoed `path` pointer.
- On failure: `f` is left closed (`handle == NULL`, `is_open == false`) and an `Error` is returned. No handle is leaked.
- Calling `File_open` on an already-open object is **not supported** in phase 1: it resets the object and loses the previous handle. Close first, then reopen.

## `path` is borrowed

`File_open` does not copy `path`. The caller's pointer must remain valid for the entire lifetime the `File` is open. Do not pass a stack buffer that goes out of scope before `File_close`.

## Who closes

The caller that opened the file is responsible for calling `File_close(&f)` exactly once per successful open, on every path including error and early return. `File_close` on an already close object is a no-op and returns `ERROR_OK` (safe to call twice).