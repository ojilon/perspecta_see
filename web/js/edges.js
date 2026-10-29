/* edges.js — draw the links.
 *
 * Two graphs share one canvas:
 *
 *   files mode   one line per file-to-file link (include, <script>, stylesheet,
 *                import, css class, html id, api route, page link). Aggregated
 *                links carry a ×n badge.
 *   calls mode   one line per function-to-function call, coloured by what it
 *                crosses: same file = solid blue, cross file = dashed violet,
 *                recursion = dashed green.
 *
 * Selecting a function always adds *its* links on top, labelled "calls" and
 * "used by", and fades everything that is not involved. That is the "function
 * scope" view from the concept art, without needing a mode switch.
 */

const linesEl = $("lines");
const MAX_CALL_LINES = 1200;

/* ---------- markers (arrow heads) ---------- */
function markerId(name) { return "m-" + name.replace(/[^a-z0-9]/gi, "-"); }

function addMarkers(svg) {
  const defs = svgEl("defs", {});
  const add = (name, color) => {
    const m = svgEl("marker", {
      id: markerId(name), viewBox: "0 0 10 10", refX: "9", refY: "5",
      markerWidth: "7", markerHeight: "7", orient: "auto-start-reverse",
    });
    m.append(svgEl("path", { d: "M0,0 L10,5 L0,10 z", fill: color }));
    defs.append(m);
  };
  Object.keys(LINK).forEach((k) => add(k, LINK[k].color));
  Object.keys(CALL_STYLE).forEach((k) => add("call-" + k, CALL_STYLE[k].color));
  svg.append(defs);
}

/* ---------- geometry ---------- */
function cardWidth(c) { return c.el.offsetWidth; }
function cardHeight(c) { return c.el.offsetHeight; }

/* Vertical position inside the card for a link: a function row if we have one,
   otherwise spread the file-level links down the card edge. */
function anchorY(c, symId, slot) {
  if (symId !== null && symId !== undefined) {
    const row = c.rows.get(symId);
    if (row) return c.y + row.offsetTop + row.offsetHeight / 2;
    return c.y + 26;
  }
  const h = Math.max(30, cardHeight(c));
  return c.y + 16 + ((slot * 7) % Math.max(24, h - 26));
}

/* A curved line from card a (at ay) to card b (at by). */
function linkPath(a, ay, b, by, srcSlot) {
  const aw = cardWidth(a), bw = cardWidth(b);
  let x1, x2, c1, c2;
  if (b.x >= a.x + aw) {                        /* b is to the right */
    x1 = a.x + aw; x2 = b.x;
    const dx = Math.max(46, (x2 - x1) / 2);
    c1 = x1 + dx; c2 = x2 - dx;
  } else if (b.x + bw <= a.x) {                 /* b is to the left */
    x1 = a.x; x2 = b.x + bw;
    const dx = Math.max(46, (x1 - x2) / 2);
    c1 = x1 - dx; c2 = x2 + dx;
  } else {                                      /* same column: bow out right */
    x1 = a.x + aw; x2 = b.x + bw;
    c1 = c2 = Math.max(x1, x2) + 60 + (srcSlot % 3) * 18;
  }
  return {
    d: "M" + x1 + "," + ay + " C" + c1 + "," + ay + " " + c2 + "," + by + " " + x2 + "," + by,
    mx: (x1 + 3 * c1 + 3 * c2 + x2) / 8,
    my: (ay + by) / 2,
    sx: x1, sy: ay, ex: x2, ey: by,
  };
}

/* Recursion: a loop that leaves and re-enters the same row. */
function loopPath(a, y) {
  const x = a.x + cardWidth(a);
  const w = 46;
  return {
    d: "M" + (x - 6) + "," + y + " C" + (x + w) + "," + (y - 34) + " " + (x + w) + "," + (y + 34) + " " + (x - 6) + "," + y,
    mx: x + w * 0.78, my: y,
    sx: x - 6, sy: y, ex: x - 6, ey: y,
  };
}

/* ---------- decorations ---------- */
function drawBadge(svg, x, y, text) {
  const w = 14 + text.length * 6.4;
  svg.append(svgEl("rect", { class: "badge-bg", x: x - w / 2, y: y - 8.5, width: w, height: 17, rx: 8.5 }));
  const t = svgEl("text", { class: "badge-text", x, y: y + 0.5 });
  t.textContent = text;
  svg.append(t);
}

function drawLabel(svg, x, y, text) {
  const w = 8 + text.length * 5.9;
  svg.append(svgEl("rect", { class: "edge-label-bg", x: x - w / 2, y: y - 8, width: w, height: 16, rx: 6 }));
  const t = svgEl("text", { class: "edge-label", x, y: y + 0.5 });
  t.textContent = text;
  svg.append(t);
}

function drawJunction(svg, x, y, color) {
  svg.append(svgEl("circle", { class: "junction", cx: x, cy: y, r: 3.4, stroke: color }));
}

/* Dashed rectangle around the cards that share a folder (one column only). */
function drawZones(svg) {
  const dirs = new Map();
  G.cards.forEach((c) => {
    const dir = dirname(c.f.path) || "(root)";
    if (!dirs.has(dir)) dirs.set(dir, []);
    dirs.get(dir).push(c);
  });
  dirs.forEach((cs, dir) => {
    if (cs.length < 2) return;
    const col = (LANG[cs[0].f.lang] || LANG.other).col;
    if (cs.some((c) => (LANG[c.f.lang] || LANG.other).col !== col)) return;  /* mixed: skip */
    let x0 = Infinity, y0 = Infinity, x1 = -Infinity, y1 = -Infinity;
    cs.forEach((c) => {
      x0 = Math.min(x0, c.x); y0 = Math.min(y0, c.y);
      x1 = Math.max(x1, c.x + cardWidth(c)); y1 = Math.max(y1, c.y + cardHeight(c));
    });
    const pad = 13;
    svg.append(svgEl("rect", {
      class: "zone", x: x0 - pad, y: y0 - pad - 15,
      width: x1 - x0 + 2 * pad, height: y1 - y0 + 2 * pad + 15, rx: 11,
    }));
    const t = svgEl("text", { class: "zone-label", x: x0 - pad + 7, y: y0 - pad - 3 });
    t.textContent = dir + "/";
    svg.append(t);
  });
}

function edgeEl(svg, path, color, dash, marker, classes) {
  const el = svgEl("path", {
    class: "edge " + (classes || ""),
    d: path.d,
    stroke: color,
    "marker-end": "url(#" + markerId(marker) + ")",
  });
  if (dash) el.setAttribute("stroke-dasharray", dash);
  svg.append(el);
  return el;
}

/* ---------- the two modes ---------- */

function drawFileEdges(svg) {
  const slot = new Map();
  const next = (c) => { const n = slot.get(c) || 0; slot.set(c, n + 1); return n; };
  G.data.edges.forEach((e) => {
    const a = G.cards.get(e.from), b = G.cards.get(e.to);
    if (!a || !b) return;
    const st = styleOf(e.kind);
    const sa = next(a), sb = next(b);
    const p = linkPath(a, anchorY(a, null, sa), b, anchorY(b, null, sb), sa);
    const el = edgeEl(svg, p, st.color, st.dash, e.kind);
    el.dataset.from = e.from;
    el.dataset.to = e.to;
    if (e.n > 1) drawBadge(svg, p.mx, p.my, "×" + e.n);
  });
}

function callStyle(l, sa, sb) {
  if (l.from === l.to) return CALL_STYLE.recur;
  if (sa.file === sb.file) return CALL_STYLE.same;
  return CALL_STYLE.cross;
}

function drawCallEdges(svg) {
  const calls = G.data.calls;
  let drawn = 0;
  calls.forEach((l) => {
    if (drawn >= MAX_CALL_LINES) return;
    const sa = G.syms.get(l.from), sb = G.syms.get(l.to);
    if (!sa || !sb) return;
    const a = G.cards.get(sa.file), b = G.cards.get(sb.file);
    if (!a || !b) return;
    const st = callStyle(l, sa, sb);
    const opacity = l.kind === "mcall" ? ".32" : ".62";
    if (l.from === l.to) {
      const p = loopPath(a, anchorY(a, l.from, 0));
      edgeEl(svg, p, st.color, st.dash, "call-" + styleKey(l, sa, sb), "loop")
        .setAttribute("opacity", opacity);
    } else {
      const p = linkPath(a, anchorY(a, l.from, 0), b, anchorY(b, l.to, 0), 0);
      edgeEl(svg, p, st.color, st.dash, "call-" + styleKey(l, sa, sb))
        .setAttribute("opacity", opacity);
    }
    drawn++;
  });
}

function styleKey(l, sa, sb) {
  if (l.from === l.to) return "recur";
  return sa.file === sb.file ? "same" : "cross";
}

/* The links of the selected function, drawn last so they sit on top. */
function drawSelection(svg) {
  const sel = G.selSym;
  const involved = new Set([sel]);
  G.data.calls.forEach((l) => {
    if (l.from !== sel && l.to !== sel) return;
    const sa = G.syms.get(l.from), sb = G.syms.get(l.to);
    if (!sa || !sb) return;
    const a = G.cards.get(sa.file), b = G.cards.get(sb.file);
    if (!a || !b) return;
    const st = callStyle(l, sa, sb);
    const key = "call-" + styleKey(l, sa, sb);
    if (l.from === l.to) {
      const p = loopPath(a, anchorY(a, l.from, 0));
      edgeEl(svg, p, st.color, st.dash, key, "loop hot");
      drawLabel(svg, p.mx, p.my - 18, "recursion");
      return;
    }
    const p = linkPath(a, anchorY(a, l.from, 0), b, anchorY(b, l.to, 0), 0);
    edgeEl(svg, p, st.color, st.dash, key, "hot");
    drawJunction(svg, p.sx, p.sy, st.color);
    drawJunction(svg, p.ex, p.ey, st.color);
    drawLabel(svg, p.mx, p.my, l.from === sel ? "calls" : "used by");
    involved.add(l.from);
    involved.add(l.to);
  });
  return involved;
}

/* Dim the cards and rows that are not part of the selection. */
function updateFocus(involved) {
  const sel = G.selSym;
  const files = new Set();
  if (sel !== null) {
    files.add(G.syms.get(sel).file);
    if (involved) involved.forEach((id) => { const s = G.syms.get(id); if (s) files.add(s.file); });
  }
  const editing = E.path ? [...G.files.values()].find((f) => f.path === E.path) : null;
  G.cards.forEach((c, id) => {
    const isEditing = editing && editing.id === id;      /* never dim what you are typing in */
    c.el.classList.toggle("dim", sel !== null && !files.has(id) && !isEditing);
    c.el.classList.toggle("selected", sel !== null && files.has(id));
    c.rows.forEach((row, sid) => {
      row.classList.toggle("sel", sid === sel);
      row.classList.toggle("hl", sid !== sel && involved && involved.has(sid));
    });
  });
}

let edgeFrame = 0;

/* Redrawing hundreds of paths on every mousemove is wasteful; during a drag we
   ask for one redraw per animation frame instead. */
function scheduleEdges() {
  if (edgeFrame) return;
  edgeFrame = requestAnimationFrame(() => {
    edgeFrame = 0;
    drawEdges();
  });
}

/* Hovering a card lights up the file links that touch it. */
function highlightFile(id) {
  linesEl.querySelectorAll("path.edge").forEach((el) => {
    if (!el.dataset.from) return;                     /* call lines have no file pair */
    const on = Number(el.dataset.from) === id || Number(el.dataset.to) === id;
    el.classList.toggle("hot", id !== null && on);
    el.classList.toggle("faded", id !== null && !on);
  });
}

function clearHighlight() { highlightFile(null); }

function drawEdges() {
  const svg = linesEl;
  svg.textContent = "";
  addMarkers(svg);
  if (!G.data) return;
  drawZones(svg);
  if (G.mode === "calls") drawCallEdges(svg);
  else drawFileEdges(svg);
  let involved = null;
  if (G.selSym !== null) involved = drawSelection(svg);
  updateFocus(involved);
}
