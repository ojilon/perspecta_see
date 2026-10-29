# The parser, and what it cannot see

There is no grammar here. The parser does what a person does when skimming a codebase:
first cross out the prose, then look for the few shapes that carry meaning. Every rule
below is short on purpose, so that when it gets something wrong you can find and change
the rule instead of negotiating with a framework.

Everything runs on a **masked copy** of the file: same length, same newlines, comments or
strings replaced by spaces. That is why every line number the UI shows is exact, and why
one scan can share its results with another without an offset table.

## 1. Masking (`src/mask.c`)

| Function | Blanks | Keeps | Used by |
| --- | --- | --- | --- |
| `mask_comments` | comments | strings | the string scans (routes, ids, includes) |
| `mask_code` | comments **and** string contents | newlines, quotes | the code walker |
| `mask_preproc` | C preprocessor lines (with `\` continuations) | — | the code walker |

The split matters. A route *is* a string (`"/api/scan"`), so the route scan needs strings
intact but comments blanked. A call is *not* a string, so the walker must blank string
contents too — otherwise `if (c == "{")` opens a block that never closes.

Before this split, both kinds of scan read the raw text, and any comment that happened to
mention `/api/scan` became a second route node, and `getElementById("tree")` inside a
comment became a DOM link. Running the scans on `mask_comments` output removes the whole
class of false positives. (Strings inside comments are blanked along with the comment, and
`//` inside a string is preserved, so `"http://x"` is not mistaken for a comment.)

## 2. String scans (`src/patterns.c`)

Find a prefix, read the quoted literal after it. Line numbers come from a pointer walk, so
the scan is O(text) with no allocation.

| Pattern | Meaning |
| --- | --- |
| `#include "x.h"` | file link (angle brackets are system headers, ignored) |
| `"`…`/api/`…`"` | in C: route *definition*; in JS: route *use* |
| `getElementById("x")` | DOM id |
| `$("x")`, `$("#x")`, `$(".x")` | DOM id / css class — this project's own shorthand |
| `querySelector("#x .y")`, `querySelectorAll(...)` | first selector term only |
| `classList.add("x")` | css class |
| `require("./x")`, `import("./x")`, `import ... from "./x"` | relative module only |

Adding a rule is one line in `scan_dom_refs()` (or `scan_routes()`), plus a mode if it
reads something other than a quoted literal.

## 3. Code walker (`src/parse_code.c`)

Input: comment- and string-masked text. Output: function symbols with their line range,
and call references attributed to the enclosing function.

It keeps a stack of open bodies: every `{` pushes `(depth, symbol)` and the matching `}`
closes it. Two heuristics find definitions:

* **`name(...)` followed by `{`** defines something. In C only at top level; in JS also
  inside a class or object literal — that is how `draw() { … }` methods are picked up.
* JS-specific: `function name(...)`, and at top level `const name = (...) => {`,
  `const name = function (...)`, `const name = x => {` (cleared again on `;`, so an
  expression-bodied arrow does not swallow the next block).

Inside a body, `name(` is a call. `obj.name(` is recorded separately as `mcall`, and a
member call is only ever resolved **inside its own file** — a member name is weak evidence,
so it earns a weak link. Recursion (`from == to`) is kept: it is drawn as a loop.

## 4. HTML (`src/parse_html.c`)

One walk over the tags records `id`, `class="a b c"`, `<script src>`, `<link href>`,
`<a href>` and the folder/name of every element. Inline `<script>` and `<style>` bodies are
noted as spans; afterwards each body is parsed by the normal JS or CSS pipeline on a copy
of the page in which everything *except* that span is blanked.

That last trick is why inline code needs no offset arithmetic: the JS parser sees a
document of the same length and shape, so "line 42" is line 42 of the html file. A page
that keeps all its JavaScript in one file still gets function-level links.

## 5. CSS (`src/parse_css.c`)

A tiny block stack distinguishes declaration blocks from at-rule groups. A `.name` or
`#name` in *selector* position becomes a class/id symbol; `@import "x.css"` becomes a
reference. Inside declaration blocks nothing is recorded (values are not links).

## 6. Resolution (`src/resolve.c`)

Runs once, after every file has been read:

```
same file  ►  same family (C+H, or JS incl. inline <script>)  ►  any file
```

* Paths: relative to the *mentioning* file, with `.`/`..` folded; `/x` means "from the
  scanned root"; include paths also try "ends with this suffix" for include folders.
* Symbols: ids are looked up in HTML files, classes in CSS files, routes anywhere,
  functions in the caller's family.
* Misses: file paths and `/api` routes are reported in `unresolved`. Missing classes and
  DOM ids are **not** — those are frequently built at runtime (`"row-" + id`), and the list
  would be noise instead of a to-do list.

## What it cannot see (deliberately)

* macros that generate function definitions (`#define DEF_FN(n) …`);
* `obj.method()` calls that leave their own file;
* class methods reached through `this.…` in another file, prototypes, mixins;
* DOM ids/classes assembled at runtime, `innerHTML` templates;
* dynamic `import(variable)`, bundler aliases, webpack/require plugins;
* C function pointers, `goto`, anything about types;
* JSX/TS/TSX (not scanned at all — only `.c .h .js .mjs .html .htm .css`).

The `unresolved` list (`G.data.unresolved` in the console, or the red pill in the HUD) is
the honest measure of what fell through.

## Extending it

**A new kind of string link** — add an `R_*` value in `include/model.h`, its name in the
`KIND[]` table in `src/json.c`, a colour in `web/js/state.js`, and a scan line in
`src/patterns.c`. Nothing else needs to know.

**A new language** — add to `lang_of()` (`src/fs.c`), the `LANG[]` name in `src/json.c`,
a branch in `parse_file()` (`src/parse.c`), and a column in `web/js/state.js`.

**Fixing a false positive** — look at the mask first. If the offending text is prose, it
should already have been blanked; if it is code that only *looks* like a link, the fix
belongs in the relevant scan function.
