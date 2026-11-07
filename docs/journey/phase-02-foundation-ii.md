# Phase 2: Foundation II (~4 weeks)

Arenas, a string type (pointer plus length), safe string operations, and a
dynamic array. Hand-write each one. Compare with how the RAD Debugger's base
layer does it.

## Goal
Memory and string primitives so later phases (scanner, tokenizer) do not keep
re-inventing buffer management. No hidden allocation: functions that need memory
either take a caller-provided buffer or explicitly allocate through the arena.

## Why it matters
C string bugs (missing NUL, buffer overruns, use-after-free) are the easiest way
to lose a whole week of sessions. A string type with explicit length and an
arena that frees everything at once removes most of them up front.

## Prerequisites
- Phase 1 definition of done: `File` API, `build.bat`/`build.sh`, `types.h`,
  test program passing.

## Sessions

### Session 1 — arena header ([hand-written])
**Goal.** An arena: one pointer, one size, one `free` at the end.
**Shape shown first.**
```c
typedef struct {
    u8 *buffer;
    usize size;
    usize used;
} Arena;

Arena arena_create(usize capacity);
void *arena_alloc(Arena *self, usize size);
void arena_free(Arena *self);
```
**Expected result.** `src/arena.h` compiles; nothing calls it yet.
**Stop here.** Declaration compiles. Body next session.

### Session 2 — arena body ([hand-written])
**Goal.** Bump-pointer allocation: return `buffer + used`, advance `used`.
**Shape shown first.**
```c
void *arena_alloc(Arena *self, usize size) {
    void *p = self->buffer + self->used;
    self->used += size;
    return p;
}
```
**Expected result.** Compiles; no caller yet.
**Stop here.** `arena_alloc` returns a pointer inside the buffer; bounds check returns NULL when full.

### Session 3 — visible result: arena test ([hand-written])
**Goal.** Allocate several items from one arena, then free once.
**Shape shown first.** `tests/arena_test.c`: `arena_create(1024)`, allocate an `i32` and a `u8[16]`, print the two addresses, `arena_free`.
**Expected result.** Program prints two addresses inside the arena buffer and exits 0.
**Stop here.** Allocations succeed; single `arena_free` at the end.

### Session 4 — string type ([hand-written])
**Goal.** `String` = pointer + length, no hidden NUL requirement.
**Shape shown first.**
```c
typedef struct {
    const char *data;
    usize length;
} String;

#define STRING_LITERAL(s) { s, sizeof(s) - 1 }
```
**Expected result.** `src/string.h` compiles.
**Stop here.** Type compiles. Operations next session.

### Session 5 — safe string operations: equality and copy ([hand-written])
**Goal.** `string_equals`, `string_copy_into` (caller-provided buffer).
**Shape shown first.**
```c
bool string_equals(String a, String b);
usize string_copy_into(String src, char *dst, usize dst_size);
```
**Expected result.** Compiles; no caller yet.
**Stop here.** Both functions written; bounds-checked copy never writes past `dst_size`.

### Session 6 — visible result: compare two strings ([hand-written])
**Goal.** Use `string_equals` to test whether a path ends in `.c`.
**Shape shown first.** Build a `String` from a file path, slice the last 2 characters, `string_equals(slice, STRING_LITERAL(".c"))`, print YES/NO.
**Expected result.** Program prints YES for `src\main.c`, NO for `README.md`.
**Stop here.** Visible YES/NO on screen.

### Session 7 — dynamic array header ([hand-written])
**Goal.** A growable array of pointers with explicit count and capacity.
**Shape shown first.**
```c
typedef struct {
    void **items;
    usize count;
    usize capacity;
} Array;

Error array_init(Array *self, usize initial_capacity);
Error array_push(Array *self, void *item);
void array_free(Array *self);   /* frees the items array, not the items */
```
**Expected result.** `src/array.h` compiles.
**Stop here.** Declaration compiles. Body next session.

### Session 8 — dynamic array body ([hand-written])
**Goal.** `array_push` grows by doubling when full.
**Shape shown first.** If `count == capacity`, `realloc` to `capacity * 2`, then store item at `items[count++]`.
**Expected result.** Compiles; caller in next session.
**Stop here.** Push beyond initial capacity works without crashing.

### Session 9 — visible result: collect lines ([hand-written])
**Goal.** Read `src\main.c` line by line into an array, print line count.
**Shape shown first.** `File_read_whole`, then split on `\n`, `array_push` each line start into an `Array`, print `array.count`.
**Expected result.** Program prints `lines: 135` (matching the line counter from Phase 1).
**Stop here.** Line count matches Phase 1's `Count_lines` output.

### Session 10 — reading session: RAD Debugger base layer ([reading])
**Goal.** Compare your arena/string/array with a real one.
**What to read.** The RAD Debugger repo: its base layer first (look for `src/base/`, `base/`, or similar — verify the exact path). Spend 30 minutes, write 5 notes on how it handles allocation, strings, or errors.
**Expected result.** 5 notes in your session log.
**Stop here.** Notes written; no code written.

### Session 11 — ownership rule: who frees what ([hand-written])
**Goal.** Write down the rule in `docs/guides/code-style.md` and follow it in code.
**Shape shown first.** Comment block at the top of `arena.h`:
`/* Arena owns its buffer. Caller frees the arena, never individual allocations. */`
**Expected result.** Rule documented; code matches it.
**Stop here.** Every allocation has exactly one free path.

### Session 12 — no-hidden-allocation audit ([hand-written])
**Goal.** Check that no string/array function allocates secretly.
**Shape shown first.** Each function either takes an arena or a caller buffer; anything that mallocs is named `create`/`new` and documented.
**Expected result.** Audit notes in session log; functions renamed if they violated the rule.
**Stop here.** Allocation ownership is obvious from each function's name/signature.

### Session 13 — warnings cleanup ([hand-written])
**Goal.** `-Wall -Wextra -Wpedantic` clean again.
**Expected result.** Zero warnings.
**Stop here.** Clean build.

### Session 14 — buffer / catch-up
**Stop here.** Sessions 1–13 all compile and run.

### Session 15 — phase checkpoint
**Stop here.** Checklist below is complete.

## Deliverable
`src/arena.h/.c`, `src/string.h/.c`, `src/array.h/.c`; tests for each; one
visible result per data structure; arena-vs-RAD-Debugger notes in session log.

## Definition of done
- [ ] Arena: create/alloc/free, tested, one free at end
- [ ] String type + `string_equals` + bounded `string_copy_into`
- [ ] Dynamic array: init/push/free, tested with line collection
- [ ] Line count from array matches Phase 1's `Count_lines`
- [ ] Allocation rules documented (`who frees what`, no hidden allocation)
- [ ] RAD Debugger base-layer notes written
- [ ] Clean build with warnings on

## What to read
- `docs/guides/code-style.md` — confirm naming/ownership conventions.
- RAD Debugger base layer (one file, 30 minutes).

## Buffer
One buffer week before Phase 3.
