# Phase 9: Code generation (~3 weeks)

A small C generator producing enum-to-string tables and struct dumpers for your
own code, so the project builds part of itself.

## Goal
`perspecta gen enum <file.h>` emits a C file with `const char *name_of(MyEnum e)`;
`perspecta gen dump <file.h>` emits a function that prints a struct's fields.
Self-generating code is a deliberate theme of this project.

## Why it matters
Boilerplate (enum-to-string, struct printing) is tedious to write by hand and
exactly the kind of thing a small generator handles well. Generating code for
your own projects is dogfooding at the meta level: perspecta_see builds part of
itself.

## Prerequisites
- Phase 8 definition of done (check command).
- Tokenizer from Phase 4 must handle `enum` and `struct` blocks.

## Sessions

### Session 1 — gen command skeleton ([hand-written])
**Goal.** `perspecta gen enum <file.h>` prints the file name.
**Expected result.** `perspecta gen enum foo.h` prints `gen enum: foo.h`.
**Stop here.** Command parses.

### Session 2 — enum detection ([hand-written])
**Goal.** From tokens, find `enum Name { A, B, C };` and list the enumerators.
**Shape shown first.** On `TOK_KEYWORD` `enum`, read name identifier, then identifiers separated by `,` until `}`.
**Expected result.** `enum Color { RED, GREEN, BLUE };` → prints RED, GREEN, BLUE.
**Stop here.** Enumerator names extracted.

### Session 3 — enum-to-string generator ([hand-written])
**Goal.** Emit C code:
```c
const char *color_name(Color c) {
    switch (c) { case RED: return "RED"; case GREEN: return "GREEN"; ... }
}
```
**Shape shown first.** Print `switch` with one `case` per enumerator.
**Expected result.** Generated `.c` file compiles when combined with the enum header.
**Stop here.** Generated file compiles.

### Session 4 — `gen enum` on your own code ([hand-written])
**Goal.** Generate enum-to-string for an enum in one of your projects.
**Expected result.** Generated file exists and compiles in that project.
**Stop here.** Dogfooding done.

### Session 5 — struct detection ([hand-written])
**Goal.** Find `struct Name { type field; ... };` and list fields with types.
**Shape shown first.** On `struct`, read name, then `type name;` pairs until `}`.
**Expected result.** `struct Point { int x; int y; };` → fields printed.
**Stop here.** Fields extracted.

### Session 6 — struct dumper generator ([hand-written])
**Goal.** Emit:
```c
void point_dump(Point *p) {
    printf("Point { x: %d, y: %d }\n", p->x, p->y);
}
```
**Expected result.** Generated dumper compiles and prints a struct instance.
**Stop here.** Dumper works on a test struct.

### Session 7 — `gen dump` on your own code ([hand-written])
**Goal.** Generate a struct dumper for one of your projects.
**Expected result.** Generated file exists and compiles in that project.
**Stop here.** Dogfooding done.

### Session 8 — reading session: code generation in C projects ([reading])
**Goal.** See how real projects generate code.
**What to read.** Look for a `gen/`, `tools/`, or `scripts/` folder that produces `.c`/`.h` (your repos, RAD Debugger, or Odin — verify), 30 minutes, 5 notes on how generation is triggered (build script? Makefile? manual?).
**Expected result.** 5 notes in session log.
**Stop here.** Notes written.

### Session 9 — generator for perspecta_see itself ([hand-written])
**Goal.** Generate something used by perspecta_see's own code (e.g. token-kind name table).
**Expected result.** `src/` contains a generated file included in the build.
**Stop here.** Project builds part of itself.

### Session 10 — warnings cleanup ([hand-written])
**Stop here.** Zero warnings.

### Session 11 — buffer / catch-up
**Stop here.** Sessions 1–9 complete.

### Session 12 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
`perspecta gen enum` and `perspecta gen dump` generate compilable C; used on
your own projects and on perspecta_see itself.

## Definition of done
- [ ] Enum detection from tokens
- [ ] Enum-to-string generator emits compilable C
- [ ] Struct detection from tokens
- [ ] Struct dumper generator emits compilable C
- [ ] Used on your own projects
- [ ] Used on perspecta_see itself (one generated file in the build)
- [ ] Clean build with warnings on

## What to read
- `docs/guides/studying-codebases.md` — for the code-generation reading session.
- Articles about code generation in C (verify title/link; search "C code generation boilerplate").

## Honest limit
Generators work on token patterns, not full types. Complex enums (nested,
bitfields) and complex structs (nested types, function pointers) are skipped
this year. The generated code must compile — that is the only acceptance test.

## Buffer
One buffer week before Phase 10.
