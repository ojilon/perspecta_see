## Session metadata
- Date: 2026-11-25
- Phase: 1
- Session: 6 (build entry points)
- Time spent: 20 min

## What I did
- Created `build.bat`: `gcc -std=c11 -Wall -Wextra -Wpedantic -o perspecta.exe src\main.c src\error.c src\file.c`
- Created `build.sh`: `gcc -std=c11 -Wall -Wextra -Wpedantic -o perspecta src/main.c src/error.c src/file.c`
- Ran the script on the current machine; confirmed `perspecta.exe` appears and runs.
- Marked the other platform (Termux) as unverified until actually tested.

## What I learned
- One command builds the whole project on this Windows machine; zero warnings with -Wall -Wextra -Wpedantic.

## What is next
- Add `File_read_chunk` signature to file.h, then body in file.c (caller-provided buffer).

## State
- [x] Compiles clean with build.bat / build.sh
- [x] Visible result confirmed on screen (binary starts, usage message)
- [ ] Session log committed