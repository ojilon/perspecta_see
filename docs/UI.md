# The map: what you are looking at

## Nodes

One card per source file. Reading a card top to bottom:

```
┌──────────────────────────────────────────┐
│ C   scan.c                        71 ln │  language · name · size
│ src                                      │  folder it lives in
│ ──────────────────────────────────────── │
│ ◆ /api/scan            35–39             │  route rows are green, diamond
│ parse_file             75–93             │  one row per function + scope
│ scan_to_json           41–69             │
│ +4 more   2 ids   1 class                │  what did not fit, ids/classes
│ 2 dangling                               │  references this file could not match
└──────────────────────────────────────────┘
```

* **Badge** — language, colour-coded (C/H blue, JS yellow, HTML orange, CSS violet).
* **Rows** — functions and api routes with their `line–end` scope. A route's `end` equals
  its line because a route is a string, not a body.
* **`+n more`** — cards show 14 rows; the rest are counted, not hidden from the scan.
* **`n dangling`** in red — this file has references that matched nothing. Hover to see
  them listed in the HUD (bottom left).

Cards live in columns by language (html → css → js → c/h), and a new column starts when
the current one gets taller than ~1000 px, so "fit" never zooms out to nothing. Drag a
header to move a card; positions are remembered per project, and **Tidy** forgets them.

## Links

| Line | Kind |
| --- | --- |
| solid blue | `#include` |
| solid orange | `<script src>` |
| dashed violet | stylesheet / `import` |
| dotted yellow | DOM id (JS → HTML) |
| dotted lilac | css class (HTML/JS → CSS) |
| solid green | `/api/…` route (JS → C) |
| dotted amber | `<a href>` page link |
| solid blue (calls mode) | function → function, same file |
| dashed violet (calls mode) | function → function, cross file |
| dashed green (calls mode) | recursion (a loop off the right edge of the row) |

An aggregated file link carries a `×n` badge: how many references agree. In calls mode,
method calls are drawn at 45 % opacity — they are weaker evidence (see `docs/PARSER.md`).

## Modes

**Files** draws one line per file pair. **Calls** draws one line per function pair, which
is where the "function scope" view of the concept art lives. Selecting a function row works
in *both* modes: its own links are drawn on top with `calls` / `used by` labels and a
junction dot at each end, and everything else is dimmed.

## The editor, inside the node

Clicking a card header (or its `⤢` button) expands **that card** into an editor. Nothing is
moved, nothing is covered by a new window: the file's links stay attached to it, its
folders and neighbours stay where they were.

* Gutter line numbers, its own scrollbar, horizontal scroll for long lines.
* Double-click a function row: the editor opens and the function body is highlighted
  (`line..end` from the parser) — the row you clicked and the code you got are visibly the
  same object.
* `Ctrl+S` saves and re-scans; the editor reopens at the same scroll position. A `●` next
  to the title and in the tree row means unsaved changes; `saved` flashes green.
* `Tab` inserts four spaces, `Esc` closes (asking first if the file is dirty).
* One editor at a time. Opening another file asks before discarding unsaved text.
* Saving writes LF line endings (a CRLF file will look like a whole-file change in git the
  first time you edit it there).

## The left column, the HUD, the search

* **FILES** — the same files, linear, grouped by folder; folders collapse; the open file is
  highlighted. Click a row to open it in place and centre its card.
* **HUD** (bottom left) — the scan summary `files · symbols · links · calls · ms`, and, on
  selection, the function's name, file, scope and how many links go in and out, followed by
  a small **table of contents**: `calls → name (file:line)` and `used by ← name (file:line)`
  as clickable chips. Clicking a chip walks to the other end (selects the function, or opens
  the file at the line when the target is an id, a class or a route). Chasing a curve across
  a zoomed-out map is guesswork; this is the same information as a list.
  The red `n dangling` pill expands the unmatched references; click an entry to jump to
  that line.
* **Empty state** — scanning a folder with no `.c/.h/.js/.html/.css` files says so in the
  middle of the canvas instead of showing an empty map. (See `showEmpty()` in `app.js`.)
* **Legend** (bottom right) — only the kinds you can currently see, with their dash style.
* **Search** (`/`) — dims every node that does not match a file path or function name.
  `Enter` jumps to the first match.

## Mouse and keys

| Gesture | Effect |
| --- | --- |
| drag card header | move node |
| drag background | pan |
| wheel | zoom at cursor |
| click card header | open / close the inline editor |
| click function row | show its links, dim the rest |
| double-click function row | edit that function in place |
| click empty canvas / `Esc` | clear selection |
| `f` / `t` / `+` `-` / `/` | fit · tidy · zoom · search |
| `Ctrl+S` | save the open editor |

**What is remembered** (`localStorage`, per project): dragged card positions
(`perspecta.pos.v1`), the camera — pan and zoom (`perspecta.view.v1`) — and the last folder
you scanned (`perspecta.root`). So reopening a project puts you back on the same corner of
the map; **Fit** and **Tidy** overwrite the remembered camera on purpose.

The map is intentionally not a graph auto-layout: columns by language plus remembered
manual placement is predictable, and predictable beats clever when you are trying to find
your way around someone else's project.
