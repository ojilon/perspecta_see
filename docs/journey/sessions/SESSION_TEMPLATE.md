# Session log template

Copy this file for each session, e.g. `docs/journey/sessions/2026-01-15-phase-01-session-03.md`,
then fill it in. One or two lines per field is the target. Never skip the log:
it is how you resume after a gap.

## Convention

```
YYYY-MM-DD | Phase N, Session M | <one-sentence summary>

What I did:
- one line
- one line

What I learned:
- one line

What is next:
- one line (the next session's first step)
```

## Template

```
## Session metadata
- Date: YYYY-MM-DD
- Phase: N
- Session: M
- Time spent: 20 min / 45 min / ...

## What I did
- (one line per change, file, or command)

## What I learned
- (one line per insight, gotcha, or question)

## What is next
- (one line: the very first step of the next session)

## State
- [ ] Compiles clean with `build.bat` / `build.sh`
- [ ] Visible result confirmed on screen
- [ ] Session log committed
```

## Example (filled in)

```
- Date: 2026-02-03
- Phase: 1
- Session: 4
- Time spent: 25 min

## What I did
- Added `File_read_whole` to `src/file.h`
- Implemented it in `src/file.c` with `fseek`/`ftell`/`malloc` + `fread`
- Ran `perspecta.exe src\main.c`: prints `bytes read: 1352`

## What I learned
- `ftell` returns `long`, so size must be stored as `long`, not `int`
- Always check `fread`'s return against the requested byte count

## What is next
- Write a tiny `tests/file_read_test.c` that reads `src\error.h` and prints
  the first 40 bytes

## State
- [x] Compiles clean
- [x] Visible result confirmed
- [ ] Committed
```
