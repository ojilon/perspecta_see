# Architecture

The tool is a loop: **disk → parse → resolve → JSON → browser → edit → disk**.
Every stage is one small module, and every module has exactly one header in `include/`
so the dependency graph of the tool is visible in the folder listing.

```
        ┌──────────────────────────────────────────────────────────────┐
        │                         browser (web/)                       │
        │  tree · cards · edges · inline editor · api.js               │
        └───────────────▲──────────────────────────┬───────────────────┘
                        │ JSON                     │ /api/save (text)
        ┌───────────────┴──────────────────────────▼───────────────────┐
        │ src/server.c   one request at a time, GET static + /api/...  │
        │ src/api.c      ping · scan · file · save                     │
        └───────────────▲──────────────────────────┬───────────────────┘
                        │                          │
        ┌───────────────┴──────────────────────────▼───────────────────┐
        │ src/scan.c     walk the folder, read each file, time it      │
        │      │                                                       │
        │      ├─► src/parse.c     dispatch by language                │
        │      │      ├─ src/patterns.c   strings: routes, ids, includ │
        │      │      ├─ src/parse_code.c C/JS: functions, scopes, call│
        │      │      ├─ src/parse_html.c tags + inline <script>/<style│
        │      │      └─ src/parse_css.c  selectors, @import           │
        │      │      (both read src/mask.c copies of the text)        │
        │      │                                                       │
        │      ├─► src/resolve.c   mentions → links (files + symbols)  │
        │      └─► src/json.c      flat JSON for the browser           │
        └──────────────────────────────────────────────────────────────┘
                        │
                  src/store.c  the fixed arrays from include/model.h
                  src/buf.c    growable text buffer
                  src/fs.c     read/write + path allow-list
```

## Why a scan is three passes

1. **Parse** every file, recording *mentions*. At this point we know that
   `web/js/api.js` mentions `"/api/scan"`, but not whether anything defines it — the file
   that defines the route may not have been read yet (files are sorted alphabetically).
2. **Resolve** once, after everything is in memory. Mentions become links, or land in the
   dangling list. Resolution rules prefer "same file" over "same family" over "anywhere".
3. **Serialise** into one flat JSON object. The browser never asks a second question about
   the graph, so panning/zooming never touches the network.

## The data model (`include/model.h`)

All of it lives in fixed static arrays defined in `src/store.c`, so a re-scan is just a
counter reset and nothing can leak.

| Struct | One row per | Key fields |
| --- | --- | --- |
| `File` | source file under the root | `rel` (path), `lang`, `lines`, `bytes` |
| `Sym` | definition | `file`, `kind` (`func`/`route`/`id`/`class`), `name`, `line`, `end` |
| `Ref` | mention | `file`, `from_sym` (enclosing function), `kind`, `target`, `line` |
| `FEdge` | file → file link | `from`, `to`, `kind`, `n` (how many refs agree), `label` |
| `Call` | symbol → symbol link | `from`, `to`, `kind` (`call`, `mcall`, `dom`, `class`, `api`) |
| `Unres` | mention with no target | `file`, `line`, `kind`, `target` |

`Sym.end` is what makes function scopes visible: the code walker closes a symbol's range
when the matching `}` of its body is seen, which is how the UI can say
`buildCards (109–141)` and highlight exactly those lines.

## JSON contract (`include/json.h`)

```json
{ "root": "...",
  "files":      [{ "id": 0, "path": "src/api.c", "lang": "c", "lines": 81, "bytes": 2611 }],
  "syms":       [{ "id": 3, "file": 0, "kind": "func", "name": "api_handle", "line": 31, "end": 81 }],
  "edges":      [{ "from": 0, "to": 4, "kind": "include", "n": 1, "label": "fs.h" }],
  "calls":      [{ "from": 3, "to": 9, "kind": "call" }],
  "unresolved": [{ "file": 5, "line": 12, "kind": "api", "target": "/api/ghost" }],
  "stats":      { "files": 44, "syms": 291, "refs": 1264, "edges": 139,
                  "calls": 360, "unresolved": 0, "ms": 2 } }
```

Adding a field means touching three places: the struct in `include/model.h`, the writer in
`src/json.c`, and the reader in `web/js/state.js` (or the module that needs it). Unknown
`kind` strings are tolerated by the UI (they fall back to grey), so C can gain a new link
kind without breaking an older page.

## The web side

No framework and no build step: nine plain scripts loaded in dependency order.

| File | Responsibility |
| --- | --- |
| `util.js` | `$`, `make`, `svgEl`, `setStatus`, small string/path helpers |
| `api.js` | the only place that calls `fetch` |
| `state.js` | `G` (graph), `E` (the open editor), `LINK`/`CALL_STYLE` colours, saved positions |
| `tree.js` | folder column, collapse state, active-file highlight |
| `cards.js` | one card per file, column layout, the ×n "dangling" note |
| `inline-editor.js` | the editor inside the card: load, save, gutter, scope highlight |
| `edges.js` | all SVG drawing: zones, lines, labels, badges, focus dimming |
| `view.js` | pan/zoom/fit, dragging, click routing, legend |
| `app.js` | boot, `rescan`, `render`, selection, search, HUD, keys |

The canvas is one `<div id="world">` with a CSS transform. Cards are absolutely positioned
inside it; the `<svg>` is *not* scaled — it receives world coordinates, which keeps the
lines aligned at every zoom level. The textarea inside a card simply inherits the
transform, so the editor is "inside the node" for free rather than by a special case.

## Cost of the design

* Everything is rebuilt on every scan. That is fine at this size (44 files, 2 ms), and it
  removes a whole class of "stale view" bugs.
* The browser holds the entire graph, so filtering, dimming and mode switching are instant.
* One request at a time is a deliberate simplification; see `docs/ROADMAP.md`.
