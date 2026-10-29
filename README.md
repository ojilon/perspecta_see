# perspecta_see

**See how an html / css / js / c project fits together — and edit it from inside the map.**

Point the tool at a folder. It parses every source file in it and draws the project as a
map of file-nodes: what each file *offers* (functions, routes, ids, classes) and what it
*points at* (includes, scripts, stylesheets, imports, api routes, DOM ids, calls).
Click a node and its code opens **inside that node** — no separate window, no losing your
place. The first project this tool is tested on is its own source.

```
.\build.ps1 -Run                  # Windows (PowerShell)
make run                          # macOS / Linux   ->  http://127.0.0.1:8787
make test                         # 19 end-to-end checks against tests/fixture
```

Then type the folder you want to look at (or leave it: it starts on the folder the server
was launched from) and press **Scan**.

---

## What it shows

| Link | Meaning |
| --- | --- |
| `include` | C: `#include "local.h"` |
| `script` / `stylesheet` | HTML pulls in a `.js` / `.css` file |
| `import` | JS `import "./x.js"`, `require("./x")`, CSS `@import` |
| `dom id` | JS reaches an element that an HTML file defines (`getElementById`, `$("#x")`, `querySelector`) |
| `css class` | `class="card"` in HTML or `classList.add("card")` in JS, against `.card` in CSS |
| `api route` | `fetch("/api/x")` in JS against the `"/api/x"` string that the C file defines |
| `page link` | `<a href="other.html">` |
| `call` | one function calling another — same file, across files, or itself (recursion) |

Two drawing modes (top bar):

* **Files** — one line per file-to-file link. Aggregated links carry a `×n` badge.
* **Calls** — one line per function-to-function call: solid blue = same file,
  dashed violet = cross file, dashed green = recursion.

Click any function row to add *its* links on top, labelled `calls` / `used by`, with
everything unrelated dimmed. The little panel bottom-left summarises the selection, and
lists **dangling links** — references the parser could not match. That list is the most
honest description of what the parser does and does not understand.

## Editing from inside the map

* Click a **card header**: the card expands and becomes a text editor for that file,
  with line numbers and its own scrollbar. Click again (or *Close*) to fold it back.
* Double-click a **function row**: the editor opens, scrolls to that function and
  highlights its body (the `line..end` scope the C parser computed).
* **Ctrl+S / Cmd+S** saves and re-scans, keeping the editor open at the same scroll
  position. **Tab** inserts four spaces. **Esc** closes.
* The file tree on the left is the same data, linear. Clicking a file there opens it in
  place and centres its card.

## Keys and gestures

| | |
| --- | --- |
| drag card header | move the node (remembered per project, in `localStorage`) |
| drag background | pan |
| wheel | zoom around the cursor |
| `f` / `t` | fit everything / tidy (forget dragged positions) |
| `/` | filter box: dims every node that does not match a path or function name |
| `Esc` | clear the selection, then close the editor |
| `+` `-` | zoom in / out |

## How it works (short version)

```
disk ──► src/scan.c ──► src/parse.c ──► src/resolve.c ──► src/json.c ──► browser
         walk files     one pipeline     mentions become   flat JSON      web/js/*
                        per language     links
```

The server (`src/server.c` + `src/api.c`) is a tiny single-threaded HTTP server on
`127.0.0.1:8787`. Four endpoints:

```
GET  /api/ping             {ok, cwd, root}
GET  /api/scan?root=PATH   the whole graph (files, symbols, links, dangling links, timing)
GET  /api/file?path=REL    raw text
POST /api/save?path=REL    write the body back to disk
```

Only `html`, `css`, `js`, `c` and `h` files **inside the scanned folder** can be read or
written; every browser-supplied path is checked by `path_allowed()` in `src/fs.c`.

More detail:

* [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — the pipeline, module by module
* [docs/PARSER.md](docs/PARSER.md) — the heuristics, and what they cannot see
* [docs/UI.md](docs/UI.md) — what every colour, line and badge means
* [docs/ROADMAP.md](docs/ROADMAP.md) — limits, and ideas taken from the concept art

## Repository layout

```
include/     one header per module — read these first
src/         one .c per header, plus main.c
web/index.html
web/css/     base (tokens) · layout (frame) · graph (lines) · cards (nodes + editor)
web/js/      util · api · state · tree · cards · inline-editor · edges · view · app
tests/       run.sh + check.py: build, scan the fixture, exercise the API
docs/        architecture, parser notes, UI legend, roadmap
image_concepts/  the concept art this UI follows
Makefile     POSIX build        build.ps1   Windows build
```

## Limits worth knowing

* **The parser is heuristic.** It handles the common shapes of C, JS, HTML and CSS, not a
  grammar. Macros that generate functions, computed member calls and dynamic DOM ids are
  invisible; the dangling list shows what did not match. See [docs/PARSER.md](docs/PARSER.md).
* **Local use only.** The server binds `127.0.0.1` and can rewrite source files in the
  folder you typed. `--host 0.0.0.0` exists for sandboxed demos and means anyone who can
  reach the port can edit those files.
* **Single-threaded.** One request at a time; a slow scan delays the next request (scans
  of a few hundred files take a handful of milliseconds, so this has not mattered yet).
* **Line endings.** Saving from the browser writes LF. A CRLF file will show a whole-file
  diff in git the first time you edit it there.
* **Fixed tables.** Up to 1500 files, 12000 symbols, 20000 references; past that the scan
  stops recording rather than failing.

## Tests

`make test` (or `sh tests/run.sh`) copies `tests/fixture/` to a temp folder, starts the
server, and asserts the promises this README makes: comments never become routes, `$()` and
`getElementById()` do become DOM links, inline `<script>`/`<style>` are parsed with exact
line numbers, function scopes end where their body ends, a cross-file call to an inline
function resolves, save/reload round-trips, and the path guard rejects everything outside
the scanned folder. 19 checks, a couple of seconds, no dependencies beyond `python3`.
