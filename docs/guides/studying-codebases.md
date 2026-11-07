# Studying big C codebases

A method for reading the RAD Debugger, Odin, and your local GCC copy — and for
redoing a small piece in your own way and comparing.

## Step 1: Look at the shape first (10 minutes)

Before reading any logic:

- Directory tree: which folders exist (`base/`, `src/`, `include/`, `vendor/`?
  — verify what is actually there).
- Build files: Makefile, CMakeLists, or a build script — how is the project
  compiled?
- One header that defines the base types (error, memory, strings).

Write 5 notes: folder purpose, build command, entry point (`main` or `WinMain`),
naming style, error style.

## Step 2: Read one file end to end (30 minutes)

Pick one small file (< 200 lines) from the base layer. Read it once for flow,
then again with a pen:

- What does each function do? (one line per function)
- How are errors reported? (return codes, `errno`, asserts?)
- Who allocates memory, who frees it?
- What would you name it differently?

## Step 3: Redo a small piece in your own way

Copy one small idea into perspecta_see — for example the RAD Debugger's arena
allocator, or its `error_print` format. Write your version (hand-written, no
AI for the core), then compare:

| Question | Your version | The real project |
| -------- | ------------ | ---------------- |
| How are errors returned? | | |
| Who frees the buffer? | | |
| What is a type called? | | |

The goal is not to copy. The goal is to see a different answer to the same
problem, and decide which answer fits your sessions.

## Step 4: Run your tool on it

From Phase 3 onward, run `perspecta scan` / `perspecta index` on the codebase.
Record:

- Crashes (file + command)
- Wrong counts (lines, functions)
- What your tool misses that ctags/ctags-like tools find

Fix one breakage per session; log the rest for later phases.

## Specific targets

- **RAD Debugger:** base layer first (memory, strings, errors — verify exact
  directory), then one source file.
- **Odin:** one directory under `core/` or `vendor/` (verify path), then one
  source file; note macro and type style.
- **GCC (local copy):** one directory under `gcc/` (verify), then one source
  file. High-level only: how functions are organized, how macros appear,
  how the preprocessor is used. Do not try to read all of GCC.

## Notes format

Keep notes in your session log:

```
## Codebase notes: RAD Debugger, base/memory.c (verify path)
- Arena allocator: create/alloc/free, one free at end — like mine
- Error style: returns codes, no errno printing
- Naming: snake_case functions — different from my PascalCase
- Idea to copy: error messages include file:line
- Question: how do they handle out-of-memory?
```
