# Roadmap and limits

The concept art in `image_concepts/` shows a map where file nodes hold code, links are
coloured by what they cross (same folder / cross folder / error / bundled calls), and a
legend explains every line. This file records what has been adopted, what is deliberately
left out, and what would come next.

## Adopted from the concept art

| Idea | Where it lives now |
| --- | --- |
| the code editor opens **inside** the file node | `web/js/inline-editor.js`, `.card.editing` in `web/css/cards.css` |
| function scopes: `line..end` under every name | `Sym.end` (`src/parse_code.c`), rows in `web/js/cards.js`, scope highlight |
| link lines anchored to function rows, not card centres | `anchorY()` in `web/js/edges.js` |
| same-file vs cross-file vs recursion colours | `CALL_STYLE` in `web/js/state.js`, calls mode |
| `×N` badges for bundled links | `drawBadge()` in `web/js/edges.js` |
| junction dots at link ends | `drawJunction()` |
| dashed folder zones | `drawZones()` in `web/js/edges.js` |
| legend + zoom controls + fit | `#legend`, `#zoombox`, `view.js` |
| a node shows its own ids/classes/`+n more` | card footer in `web/js/cards.js` |

## Not adopted (on purpose)

* **Type-mismatch links** (`expected uint64_t, got const char*`). That needs a real
  compiler pass; a heuristic that guesses types would be confidently wrong, which is worse
  than the dangling list, which is honestly incomplete.
* **"Reserved" placeholder nodes.** They help a human sketch a system; here the layout is
  generated, so empty nodes would just be empties.
* **A second "Code" view.** The inline editor already is the code view, and it keeps the
  context the full-screen view throws away.

## Next, in rough order of value

1. **Incremental rescan.** Today every save re-parses the whole project (2 ms here, but a
   big project pays for every keystroke-free save). A mtime check per file plus a partial
   resolve would fix it without a cache invalidation story.
2. **Include-path awareness.** Resolve `#include "x.h"` against the compiler's include
   directories, so a project that does not flatten its headers stops producing dangling
   includes.
3. **Function-level search.** Filter currently matches names; matching bodies would make
   the map a code search tool too.
4. **A diff-ish save.** Show which lines changed after a save, and what links appeared or
   disappeared — the graph is already a diff of structure.
5. **Multiple open editors.** The single-editor rule is a design choice, not a technical
   limit; tab-strip support would need a small change in `state.js`.
6. **Threads.** One request at a time keeps `server.c` readable. If scans ever take longer
   than a few tens of milliseconds, the clean step is a second thread that owns the scan
   while the socket loop keeps answering.

## Limits that are not going away

* **The parser is heuristic.** `docs/PARSER.md` lists exactly what it cannot see. The
  dangling list is the interface to that incompleteness: it is meant to be read.
* **Local only.** The server can rewrite source files in the folder you typed. That is the
  feature; it is also why it binds `127.0.0.1` and why `--host` is an explicit flag.
* **Fixed tables.** 1500 files / 12000 symbols / 20000 references. Past that, scanning
  degrades (stops recording) rather than crashing.
* **LF on save.** The browser textarea cannot preserve CRLF; a CRLF file shows a whole-file
  diff the first time it is saved from the browser.
* **No auth, no TLS.** Use it on your machine, not as a shared service.
