/* graph.js: scan result -> one card per file, lines between cards, pan / zoom / drag. */

const CARD_W = 270, COL_W = 340, GAP_Y = 24, MAX_ROWS = 14;
const LANG_COL = { html: 0, css: 1, js: 2, c: 3, h: 3 };          /* columns left to right */
const COLORS = {
  include: "#5fb3ff", script: "#ff9f6b", style: "#b794ff", import: "#8f9bff",
  class: "#d6a3ff", dom: "#ffd666", api: "#4be3a0", call: "#3aa0ff",
};

/* G = everything the graph needs to remember */
const G = {
  data: null,               /* last scan result from C */
  cards: new Map(),         /* file id -> { el, f, x, y, col, rows } */
  view: { x: 40, y: 30, k: 1 },
  selSym: null,             /* selected function (symbol id) or null */
  pos: {},                  /* path -> {x, y}: remembered card positions between scans */
};
const cardsEl = $("cards"), linesEl = $("lines"), worldEl = $("world"), canvasEl = $("canvas");
const SVGNS = "http://www.w3.org/2000/svg";

function make(tag, cls, text) {
  const e = document.createElement(tag);
  if (cls) e.className = cls;
  if (text !== undefined) e.textContent = text;
  return e;
}

function svgEl(tag, attrs) {
  const e = document.createElementNS(SVGNS, tag);
  for (const k in attrs) e.setAttribute(k, attrs[k]);
  return e;
}

/* ---------- cards ---------- */
function makeCard(f, syms) {
  const el = make("div", "card lang-" + f.lang);
  el.dataset.file = f.id;

  const head = make("div", "card-head");
  head.append(make("span", "badge", f.lang.toUpperCase()),
              make("span", "name", f.path.split("/").pop()),
              make("span", "lines", f.lines + " ln"));
  el.append(head);

  const slash = f.path.lastIndexOf("/");
  if (slash >= 0) el.append(make("div", "dir", f.path.slice(0, slash)));

  const rows = make("div", "rows");
  const rowMap = new Map();
  const shown = syms.filter((s) => s.kind === "func" || s.kind === "route");
  shown.slice(0, MAX_ROWS).forEach((s) => {
    const r = make("div", "row" + (s.kind === "route" ? " route" : ""));
    r.dataset.sym = s.id;
    r.append(make("span", "fn", s.name), make("span", "ln", ":" + s.line));
    rows.append(r);
    rowMap.set(s.id, r);
  });
  el.append(rows);

  const bits = [];
  if (shown.length > MAX_ROWS) bits.push("+" + (shown.length - MAX_ROWS) + " more functions");
  const ids = syms.filter((s) => s.kind === "id").length;
  const classes = syms.filter((s) => s.kind === "class").length;
  if (ids) bits.push(ids + " ids");
  if (classes) bits.push(classes + " classes");
  if (bits.length) el.append(make("div", "more", bits.join(" · ")));

  return { el, f, x: 0, y: 0, col: 0, rows: rowMap };
}

function placeCard(c) {
  c.el.style.left = c.x + "px";
  c.el.style.top = c.y + "px";
}

function buildGraph(data) {
  G.data = data;
  G.selSym = null;
  G.cards.clear();
  cardsEl.textContent = "";

  const byFile = data.files.map(() => []);
  data.syms.forEach((s) => byFile[s.file].push(s));
  data.files.forEach((f) => {
    const c = makeCard(f, byFile[f.id]);
    cardsEl.append(c.el);
    G.cards.set(f.id, c);
  });
  layoutCards();
  drawLines();
}

/* Columns by language (html | css | js | c), cards stacked top to bottom. */
function layoutCards() {
  const cols = {};
  G.cards.forEach((c) => {
    const col = LANG_COL[c.f.lang] !== undefined ? LANG_COL[c.f.lang] : 9;
    (cols[col] = cols[col] || []).push(c);
  });
  Object.keys(cols).map(Number).sort((a, b) => a - b).forEach((col, ci) => {
    let y = 0;
    cols[col].forEach((c) => {
      const saved = G.pos[c.f.path];
      c.col = ci;
      c.x = saved ? saved.x : ci * COL_W;
      c.y = saved ? saved.y : y;
      y += c.el.offsetHeight + GAP_Y;
      placeCard(c);
    });
  });
}

/* ---------- lines ---------- */
function anchorY(c, symId, slot) {
  const r = symId !== null ? c.rows.get(symId) : null;
  if (r) return c.y + r.offsetTop + r.offsetHeight / 2;
  return c.y + 16 + (slot % 6) * 4;                       /* no row: attach near the card header */
}

/* A curved line from card a (at height ay) to card b (at height by). */
function linkPath(a, ay, b, by) {
  const aw = a.el.offsetWidth, bw = b.el.offsetWidth;
  let x1, x2, c1, c2;
  if (b.x >= a.x + aw) {                                  /* b is to the right */
    x1 = a.x + aw; x2 = b.x;
    const dx = Math.max(50, (x2 - x1) / 2);
    c1 = x1 + dx; c2 = x2 - dx;
  } else if (b.x + bw <= a.x) {                           /* b is to the left */
    x1 = a.x; x2 = b.x + bw;
    const dx = Math.max(50, (x1 - x2) / 2);
    c1 = x1 - dx; c2 = x2 + dx;
  } else {                                                /* same column: loop out the right side */
    x1 = a.x + aw; x2 = b.x + bw;
    c1 = c2 = Math.max(x1, x2) + 70;
  }
  return {
    d: "M" + x1 + "," + ay + " C" + c1 + "," + ay + " " + c2 + "," + by + " " + x2 + "," + by,
    mx: (x1 + 3 * c1 + 3 * c2 + x2) / 8,
    my: (ay + by) / 2,
  };
}

function addMarkers(svg) {
  const defs = svgEl("defs", {});
  Object.keys(COLORS).forEach((k) => {
    const m = svgEl("marker", { id: "m-" + k, viewBox: "0 0 10 10", refX: "9", refY: "5",
                                markerWidth: "7", markerHeight: "7", orient: "auto" });
    m.append(svgEl("path", { d: "M0,0 L10,5 L0,10 z", fill: COLORS[k] }));
    defs.append(m);
  });
  svg.append(defs);
}

function drawBadge(svg, x, y, text) {
  const w = 8 + text.length * 7;
  svg.append(svgEl("rect", { class: "badge-bg", x: x - w / 2, y: y - 9, width: w, height: 18, rx: 9 }));
  const t = svgEl("text", { class: "badge-text", x: x, y: y });
  t.textContent = text;
  svg.append(t);
}

/* Faint dashed rectangle around cards that share a folder (only if they sit in one column). */
function drawZones(svg) {
  const dirs = new Map();
  G.cards.forEach((c) => {
    const i = c.f.path.lastIndexOf("/");
    const dir = i < 0 ? "(root)" : c.f.path.slice(0, i);
    if (!dirs.has(dir)) dirs.set(dir, []);
    dirs.get(dir).push(c);
  });
  dirs.forEach((cs, dir) => {
    if (cs.some((c) => c.col !== cs[0].col)) return;
    let x0 = 1e9, y0 = 1e9, x1 = -1e9, y1 = -1e9;
    cs.forEach((c) => {
      x0 = Math.min(x0, c.x); y0 = Math.min(y0, c.y);
      x1 = Math.max(x1, c.x + c.el.offsetWidth); y1 = Math.max(y1, c.y + c.el.offsetHeight);
    });
    const pad = 14;
    svg.append(svgEl("rect", { class: "zone", x: x0 - pad, y: y0 - pad - 16, width: x1 - x0 + 2 * pad, height: y1 - y0 + 2 * pad + 16, rx: 10 }));
    const t = svgEl("text", { class: "zone-label", x: x0 - pad + 8, y: y0 - pad - 2 });
    t.textContent = dir + "/";
    svg.append(t);
  });
}

function drawLines() {
  const svg = linesEl;
  svg.textContent = "";
  addMarkers(svg);
  drawZones(svg);
  if (!G.data) return;

  const sel = G.selSym;
  const slots = new Map();
  const slot = (c) => { const n = slots.get(c) || 0; slots.set(c, n + 1); return n; };
  const linked = new Set();

  if (sel === null) {
    G.data.edges.forEach((e) => {                         /* file-to-file: the big picture */
      const a = G.cards.get(e.from), b = G.cards.get(e.to);
      if (!a || !b) return;
      const p = linkPath(a, anchorY(a, null, slot(a)), b, anchorY(b, null, slot(b)));
      svg.append(svgEl("path", { class: "edge", d: p.d, stroke: COLORS[e.kind], "marker-end": "url(#m-" + e.kind + ")" }));
      if (e.n > 1) drawBadge(svg, p.mx, p.my, "×" + e.n);
    });
  } else {
    G.data.calls.forEach((l) => {                         /* function-level: only links touching the selection */
      if (l.from !== sel && l.to !== sel) return;
      const sa = G.data.syms[l.from], sb = G.data.syms[l.to];
      const a = G.cards.get(sa.file), b = G.cards.get(sb.file);
      const p = linkPath(a, anchorY(a, l.from, 0), b, anchorY(b, l.to, 0));
      svg.append(svgEl("path", { class: "edge call-line", d: p.d, stroke: COLORS[l.kind], "marker-end": "url(#m-" + l.kind + ")" }));
      linked.add(l.from === sel ? l.to : l.from);
    });
  }
  updateFocus(linked);
}

/* Dim everything that is not part of the selected function's links. */
function updateFocus(linked) {
  const sel = G.selSym;
  const involved = new Set();
  if (sel !== null) {
    involved.add(G.data.syms[sel].file);
    linked.forEach((id) => involved.add(G.data.syms[id].file));
  }
  G.cards.forEach((c, id) => {
    c.el.classList.toggle("dim", sel !== null && !involved.has(id));
    c.rows.forEach((r, sid) => {
      r.classList.toggle("sel", sid === sel);
      r.classList.toggle("hl", linked.has(sid));
    });
  });
}

function selectSym(id) {
  G.selSym = G.selSym === id ? null : id;
  drawLines();
  if (G.selSym === null) return;
  const s = G.data.syms[id];
  const out = G.data.calls.filter((l) => l.from === id).length;
  const inn = G.data.calls.filter((l) => l.to === id).length;
  setStatus(s.name + "  →  uses " + out + ", used by " + inn);
}

function clearSelection() {
  if (G.selSym !== null) { G.selSym = null; drawLines(); }
}

/* ---------- view: pan, zoom, fit ---------- */
function applyView() {
  const v = G.view;
  worldEl.style.transform = "translate(" + v.x + "px," + v.y + "px) scale(" + v.k + ")";
  $("zoomlabel").textContent = Math.round(v.k * 100) + "%";
}

function fitView() {
  if (!G.cards.size) return;
  let x0 = 1e9, y0 = 1e9, x1 = -1e9, y1 = -1e9;
  G.cards.forEach((c) => {
    x0 = Math.min(x0, c.x); y0 = Math.min(y0, c.y);
    x1 = Math.max(x1, c.x + c.el.offsetWidth); y1 = Math.max(y1, c.y + c.el.offsetHeight);
  });
  const W = canvasEl.clientWidth, H = canvasEl.clientHeight;
  const k = Math.max(0.15, Math.min(1.2, (W - 80) / (x1 - x0), (H - 80) / (y1 - y0)));
  G.view.k = k;
  G.view.x = (W - (x1 - x0) * k) / 2 - x0 * k;
  G.view.y = (H - (y1 - y0) * k) / 2 - y0 * k;
  applyView();
}

function focusCard(id) {
  const c = G.cards.get(id);
  if (!c) return;
  const W = canvasEl.clientWidth, H = canvasEl.clientHeight;
  G.view.k = 1;
  G.view.x = W / 2 - (c.x + c.el.offsetWidth / 2);
  G.view.y = H / 3 - c.y;
  applyView();
}

/* ---------- mouse ---------- */
function initGraph() {
  const lg = $("legend");
  Object.keys(COLORS).forEach((k) => {
    const s = make("span");
    const i = make("i");
    i.style.background = COLORS[k];
    s.append(i, k);
    lg.append(s);
  });

  let drag = null;
  canvasEl.addEventListener("mousedown", (e) => {
    const head = e.target.closest(".card-head");
    if (head) {                                           /* drag a card by its header */
      const c = G.cards.get(Number(head.parentElement.dataset.file));
      drag = { type: "card", c, sx: e.clientX, sy: e.clientY, ox: c.x, oy: c.y, moved: false };
      e.preventDefault();
    } else if (!e.target.closest(".card")) {              /* drag the background = pan */
      drag = { type: "pan", sx: e.clientX, sy: e.clientY, ox: G.view.x, oy: G.view.y, moved: false };
    }
  });
  window.addEventListener("mousemove", (e) => {
    if (!drag) return;
    const dx = e.clientX - drag.sx, dy = e.clientY - drag.sy;
    if (Math.abs(dx) + Math.abs(dy) > 3) drag.moved = true;
    if (!drag.moved) return;
    if (drag.type === "card") {
      drag.c.x = drag.ox + dx / G.view.k;
      drag.c.y = drag.oy + dy / G.view.k;
      G.pos[drag.c.f.path] = { x: drag.c.x, y: drag.c.y };
      placeCard(drag.c);
      drawLines();
    } else {
      G.view.x = drag.ox + dx;
      G.view.y = drag.oy + dy;
      applyView();
    }
  });
  window.addEventListener("mouseup", () => {
    if (drag && !drag.moved) {
      if (drag.type === "card") openFile(drag.c.f.path);  /* a click on the header opens the file */
      else clearSelection();
    }
    drag = null;
  });
  canvasEl.addEventListener("wheel", (e) => {             /* zoom around the mouse */
    e.preventDefault();
    const r = canvasEl.getBoundingClientRect();
    const mx = e.clientX - r.left, my = e.clientY - r.top;
    const k0 = G.view.k;
    const k1 = Math.min(2.5, Math.max(0.15, k0 * (e.deltaY < 0 ? 1.1 : 1 / 1.1)));
    G.view.x = mx - (mx - G.view.x) * (k1 / k0);
    G.view.y = my - (my - G.view.y) * (k1 / k0);
    G.view.k = k1;
    applyView();
  }, { passive: false });
  cardsEl.addEventListener("click", (e) => {              /* click a function row = focus its links */
    const row = e.target.closest(".row");
    if (row) selectSym(Number(row.dataset.sym));
  });
  applyView();
}