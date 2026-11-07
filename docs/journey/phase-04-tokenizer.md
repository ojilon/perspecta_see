# Phase 4: Tokenizer (~4 weeks)

A C tokenizer: identifiers, keywords, literals, comments, preprocessor lines,
punctuation. Test on your own repos.

## Goal
`perspecta tokenize <file.c>` prints a list of tokens (type, text, line
number). Honest scope: this recognizes *lexical* tokens only. It does not
preprocess, expand macros, or resolve types — parsing full C is a multi-year
project, and heuristics are enough for year one.

## Why it matters
The tokenizer is the foundation of the indexer (Phase 5) and every diagnostic
(Phase 8). Everything downstream reads its output. Getting token boundaries
right (comments, strings, line numbers) prevents subtle bugs later.

## Prerequisites
- Phase 3 definition of done (scanner, txt/json output).

## Sessions

### Session 1 — token types header ([hand-written])
**Goal.** An enum of token kinds.
**Shape shown first.**
```c
typedef enum {
    TOK_IDENT,
    TOK_KEYWORD,
    TOK_INTEGER,
    TOK_FLOAT,
    TOK_STRING,
    TOK_CHAR,
    TOK_PUNCT,
    TOK_COMMENT,
    TOK_PREPROCESSOR,
    TOK_NEWLINE,
    TOK_EOF,
    TOK_ERROR
} TokenKind;
```
**Expected result.** `src/token.h` compiles.
**Stop here.** Enum compiles. Token struct next session.

### Session 2 — token struct + print ([hand-written])
**Goal.** Hold kind, text (pointer+length), line number; print one line.
**Shape shown first.**
```c
typedef struct {
    TokenKind kind;
    const char *start;
    usize length;
    int line;
} Token;

void token_print(const Token *t);
```
**Expected result.** Compiles; no tokenizer yet.
**Stop here.** Struct and print declared.

### Session 3 — `is_identifier_start` / `is_identifier_char` ([hand-written])
**Goal.** Character classifiers written once, used everywhere.
**Shape shown first.**
```c
bool is_id_start(char c);   /* A-Z a-z _ */
bool is_id_char(char c);    /* plus 0-9 */
```
**Expected result.** Compiles; tiny test prints true/false for `a`, `_`, `1`, `#`.
**Stop here.** Classifiers correct on paper and in test.

### Session 4 — keyword set ([hand-written])
**Goal.** Decide if an identifier is a keyword.
**Shape shown first.** Array of string literals: `auto`, `break`, `case`, `char`, `const`, `continue`, `default`, `do`, `double`, `else`, `enum`, `extern`, `float`, `for`, `goto`, `if`, `inline`, `int`, `long`, `register`, `return`, `short`, `signed`, `sizeof`, `static`, `struct`, `switch`, `typedef`, `union`, `unsigned`, `void`, `volatile`, `while`, `_Bool`, `_Complex`, `_Imaginary`. Linear search is fine.
**Expected result.** `is_keyword(String)` works.
**Stop here.** 32 keywords recognized.

### Session 5 — tokenizer skeleton ([hand-written])
**Goal.** `tokenize_file(path)` loops over characters and produces tokens.
**Shape shown first.**
```c
Error tokenize_file(const char *path, Array *out_tokens);
```
Read whole file (Phase 1), walk with an index, produce `Token`s into an `Array`.
**Expected result.** Compiles; produces no visible output yet.
**Stop here.** Function exists and compiles.

### Session 6 — identifiers and keywords ([hand-written])
**Goal.** `abc`, `_foo`, `return` become tokens.
**Shape shown first.** While `is_id_char`, consume; if `is_keyword`, kind = TOK_KEYWORD else TOK_IDENT.
**Expected result.** `perspecta tokenize src\main.c` prints identifier/keyword tokens.
**Stop here.** Visible token list for a small file.

### Session 7 — integers and punctuation ([hand-written])
**Goal.** `42`, `0xFF`, `;`, `{`, `(`, `*`, `+`, `-` become tokens.
**Shape shown first.** Digits consume digits; single non-id char becomes one TOK_PUNCT token.
**Expected result.** Numbers and punctuation print.
**Stop here.** `int x = 42;` tokenizes fully.

### Session 8 — string and char literals ([hand-written])
**Goal.** `"hello"`, `'\n'`, escaped quotes inside.
**Shape shown first.** On `"`, consume until closing `"`; on `\` inside, skip next char.
**Expected result.** Literals print without breaking on inner quotes.
**Stop here.** `"a\"b"` tokenizes as one STRING token.

### Session 9 — comments ([hand-written])
**Goal.** `//` to end of line, `/* */` blocks (nested not required).
**Shape shown first.** On `/` followed by `/`, consume to `\n`; on `/*`, consume to `*/`.
**Expected result.** Comments are TOK_COMMENT tokens; code after `//` ignored.
**Stop here.** Both comment styles work.

### Session 10 — preprocessor lines ([hand-written])
**Goal.** Lines starting with `#` become TOK_PREPROCESSOR.
**Shape son first.** At line start, if `#`, consume to end of line.
**Expected result.** `#include <stdio.h>` is one preprocessor token.
**Stop here.** Preprocessor lines recognized; no expansion (out of scope this year).

### Session 11 — line numbers ([hand-written])
**Goal.** Every token carries its line number; `\n` bumps the counter.
**Shape shown first.** Track `line` as you walk; assign to each token.
**Expected result.** `token - printf` line numbers match the file's lines.
**Stop here.** Line numbers correct on a multi-line file.

### Session 12 — reading session: your own repo ([reading])
**Goal.** Read one header of a project you own, looking for token boundaries.
**What to read.** One `.h` file you wrote, 30 minutes, write 5 notes: tricky spots for a tokenizer (macros, comments in odd places, long strings).
**Expected result.** 5 notes in session log.
**Stop here.** Notes written.

### Session 13 — visible result: tokenize your own repo ([hand-written])
**Goal.** Run `perspecta tokenize` on one file from your own code.
**Expected result.** Token list prints; note any mis-tokenized spot in the log.
**Stop here.** Dogfooding done; problems noted.

### Session 14 — warnings cleanup ([hand-written])
**Stop here.** Zero warnings.

### Session 15 — buffer / catch-up
**Stop here.** Sessions 1–14 complete.

### Session 16 — phase checkpoint
**Stop here.** Checklist below complete.

## Deliverable
`perspecta tokenize <file>` prints kind, text, line per token; tested on own
repos.

## Definition of done
- [ ] Token kinds enum + Token struct + print
- [ ] Identifiers, keywords (32), integers, floats, strings, chars
- [ ] Comments (`//` and `/* */`)
- [ ] Preprocessor lines (no expansion)
- [ ] Punctuation
- [ ] Line numbers on every token
- [ ] Tested on own repos
- [ ] Clean build with warnings on

## What to read
- `docs/guides/studying-codebases.md` — method for reading real code (used in Phase 10).
- gcc manual: preprocessor section (verify title/link) — only to understand what `#include` lines look like.

## What to skip this year
- Macro expansion, `#if` conditional compilation, full C preprocessing,
  type resolution, typedef-name recognition. These are heuristics problems for
  later phases or later years.

## Buffer
One buffer week before Phase 5.
