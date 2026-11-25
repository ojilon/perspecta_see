## Session metadata
- Date: 2026-11-25
- Phase: 1
- Session: 3 (lifecycle contract, doc only)
- Time spent: 20 min

## What I did
- Documented File lifecycle invariants: `File f = {0}` means closed; `File_open` requires a closed object; on failed open the object is left closed; repeated close is safe; `path` is borrowed and its lifetime must cover use; caller closes exactly once per successful open.
- Added the contract note to the session log (no production code change).

## What I learned
- `File_open` currently resets the object before opening, so reopening an already-open handle loses the old handle — documented as a known limitation to fix before adding callers.

## What is next
- Write a tiny lifecycle test: open real file, close twice, open missing, open on already-open object; confirm no leak / destroyed handle.

## State
- [x] Compiles clean with build.bat / build.sh
- [ ] Visible result confirmed on screen (doc only, no runnable change)
- [ ] Session log committed