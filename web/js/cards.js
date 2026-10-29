/* cards.js — one card per file, and the column layout.
 *
 * A card answers "what does this file offer?" at a glance: language, size, the
 * folder it belongs to, and one row per function or route with its scope
 * (line..end). The rows are also the anchors the link lines attach to, which is
 * why the map can draw "drawLines() calls row()" instead of just "app.js is
 * linked to graph.js".
 *
 * Positions are sticky: drag a card and it is remembered per project (G.pos,
 * saved in localStorage), so a layout you arranged survives a rescan/reload.
 */

const MAX_ROWS = 14;
const CARD_W = 262;
const COL_GAP = 110;
const ROW_GAP = 26;
const cardsEl = $("cards");

function posKey(path) { return G.root + "::" + path; }

function countBy(list, kind) { return list.filter((s) => s.kind === kind).length; }

function makeCard(f, syms) {
  const el = make("div", "card lang-" + f.lang);
  el.dataset.file = f.id;

  const head = make("div", "card-head");
  head.title = f.path + " — click to edit in place, drag to move";
  head.append(
    make("span", "badge", (LANG[f.lang] || LANG.other).label),
    make("span", "name", basename(f.path)),
    make("span", "meta", f.lines + " ln")
  );
  const edit = make("button", "edit", "⤢");
  edit.dataset.act = "edit";
  edit.title = "edit this file inside its node";
  head.append(edit);

  const dir = make("div", "card-dir", dirname(f.path) || "(root)");

  const rows = make("div", "rows");
  const rowMap = new Map();
  const shown = syms.filter((s) => s.kind === "func" || s.kind === "route");
  shown.slice(0, MAX_ROWS).forEach((s) => {
    const r = make("div", "row" + (s.kind === "route" ? " route" : ""));
    r.dataset.sym = s.id;
    r.title = s.name + "  (lines " + s.line + "–" + s.end + ")  click: show links · double-click: edit here";
    r.append(make("span", "fn", s.name));
    if (s.end > s.line) r.append(make("span", "scope", s.line + "–" + s.end));
    else r.append(make("span", "scope", ":" + s.line));
    rows.append(r);
    rowMap.set(s.id, r);
  });
  if (!shown.length) rows.append(make("div", "more", "no functions found"));

  const more = make("div", "more");
  const bits = [];
  if (shown.length > MAX_ROWS) bits.push("+" + (shown.length - MAX_ROWS) + " more");
  const ids = countBy(syms, "id");
  const classes = countBy(syms, "class");
  if (ids) bits.push(ids + (ids === 1 ? " id" : " ids"));
  if (classes) bits.push(classes + (classes === 1 ? " class" : " classes"));
  bits.forEach((b) => more.append(make("span", "", b)));
  const bad = make("span", "bad");
  more.append(bad);

  el.append(head, dir, rows, more);
  return { el, f, x: 0, y: 0, rows: rowMap, syms, issues: bad, inline: null, shown };
}

function placeCard(c) {
  c.el.style.left = c.x + "px";
  c.el.style.top = c.y + "px";
}

function buildCards(data) {
  G.cards.clear();
  cardsEl.textContent = "";

  const byFile = new Map(data.files.map((f) => [f.id, []]));
  data.syms.forEach((s) => {
    if (byFile.has(s.file)) byFile.get(s.file).push(s);
  });
  data.files.forEach((f) => {
    const c = makeCard(f, byFile.get(f.id));
    cardsEl.append(c.el);
    G.cards.set(f.id, c);
  });

  markIssues(data);
  layoutCards();
  drawEdges();
}

/* The "N dangling" note on a card: references in this file that matched nothing. */
function markIssues(data) {
  const perFile = new Map();
  data.unresolved.forEach((u) => {
    perFile.set(u.file, (perFile.get(u.file) || 0) + 1);
  });
  G.cards.forEach((c, id) => {
    const n = perFile.get(id) || 0;
    c.issues.textContent = n ? n + " dangling" : "";
    c.issues.classList.toggle("bad", n > 0);
    if (n) c.el.classList.add("has-issues");
  });
}

/* Columns by language, left to right, and a new column whenever the current
   one gets too tall. A stack of thirty C files in a single column would make
   "fit" zoom out to nothing, so the layout balances height instead. */
const COLUMN_H = 1020;

function layoutCards() {
  const groups = new Map();
  G.cards.forEach((c) => {
    const key = (LANG[c.f.lang] || LANG.other).col;
    if (!groups.has(key)) groups.set(key, []);
    groups.get(key).push(c);
  });

  let x = 0;
  [...groups.keys()].sort((a, b) => a - b).forEach((key) => {
    const list = groups.get(key).sort((a, b) => a.f.path.localeCompare(b.f.path));
    let y = 0, colX = x;
    list.forEach((c) => {
      const h = c.el.offsetHeight;
      if (y > 0 && y + h > COLUMN_H) { y = 0; colX += CARD_W + COL_GAP; }
      c.x = colX;
      c.y = y;
      const saved = G.pos[posKey(c.f.path)];
      if (saved) { c.x = saved.x; c.y = saved.y; } else { y += h + ROW_GAP; }
      placeCard(c);
    });
    x = colX + CARD_W + COL_GAP;
  });
}

/* Horizontal extent of the whole map, used by fitView. */
function mapBounds() {
  let x0 = Infinity, y0 = Infinity, x1 = -Infinity, y1 = -Infinity;
  G.cards.forEach((c) => {
    x0 = Math.min(x0, c.x);
    y0 = Math.min(y0, c.y);
    x1 = Math.max(x1, c.x + c.el.offsetWidth);
    y1 = Math.max(y1, c.y + c.el.offsetHeight);
  });
  if (!G.cards.size) return null;
  return { x0, y0, x1, y1 };
}
