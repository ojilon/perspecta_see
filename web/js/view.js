/* view.js — moving around: pan, zoom, fit, drag a card, click things.
 *
 * The canvas holds one transformed layer (#world: translate + scale). Cards are
 * plain absolutely-positioned divs inside it; the link lines live in an <svg>
 * that is never scaled, it only receives coordinates in world space, which is
 * why lines and cards always agree even while zooming.
 */

const canvasEl = $("canvas");
const worldEl = $("world");

function applyView() {
  const v = G.view;
  worldEl.style.transform = "translate(" + v.x + "px," + v.y + "px) scale(" + v.k + ")";
  $("zoomlabel").textContent = Math.round(v.k * 100) + "%";
}

/* Zoom around a point given in canvas pixels. */
function zoomAt(px, py, factor) {
  const k0 = G.view.k;
  const k1 = clamp(k0 * factor, 0.12, 2.6);
  G.view.x = px - (px - G.view.x) * (k1 / k0);
  G.view.y = py - (py - G.view.y) * (k1 / k0);
  G.view.k = k1;
  applyView();
}

function zoomBy(factor) {
  zoomAt(canvasEl.clientWidth / 2, canvasEl.clientHeight / 2, factor);
}

function fitView() {
  const b = mapBounds();
  if (!b) return;
  const W = canvasEl.clientWidth, H = canvasEl.clientHeight;
  const k = clamp(Math.min((W - 90) / (b.x1 - b.x0), (H - 90) / (b.y1 - b.y0)), 0.12, 1.15);
  G.view.k = k;
  G.view.x = (W - (b.x1 - b.x0) * k) / 2 - b.x0 * k;
  G.view.y = (H - (b.y1 - b.y0) * k) / 2 - b.y0 * k;
  applyView();
}

/* Centre a card without changing the zoom level. */
function focusCard(id) {
  const c = G.cards.get(id);
  if (!c) return;
  const W = canvasEl.clientWidth, H = canvasEl.clientHeight;
  const k = Math.max(G.view.k, 0.75);
  G.view.k = k;
  G.view.x = W / 2 - (c.x + cardWidth(c) / 2) * k;
  G.view.y = H / 2 - (c.y + Math.min(cardHeight(c), H / k) / 2) * k;
  applyView();
}

/* Pan (never zoom) so that `c` is fully visible. Used after opening a file. */
function ensureVisible(c) {
  const v = G.view;
  const W = canvasEl.clientWidth, H = canvasEl.clientHeight;
  const w = cardWidth(c) * v.k, h = Math.min(cardHeight(c), 640) * v.k;
  const x = v.x + c.x * v.k, y = v.y + c.y * v.k;
  const m = 24;
  if (x < m) v.x += m - x;
  else if (x + w > W - m) v.x -= (x + w) - (W - m);
  if (y < m) v.y += m - y;
  else if (y + h > H - m) v.y -= (y + h) - (H - m);
  applyView();
}

/* Forget dragged positions and lay the map out again. */
function tidyLayout() {
  G.pos = {};
  savePositions();
  layoutCards();
  drawEdges();
  fitView();
}

/* ---------- legend ---------- */
function legendItem(color, dash, label) {
  const k = make("span", "k");
  const line = make("i");
  line.style.borderTopColor = color;
  line.style.borderTopStyle = !dash ? "solid" : dash.startsWith("2") ? "dotted" : "dashed";
  k.append(line, document.createTextNode(label));
  return k;
}

function buildLegend() {
  const box = $("legend");
  box.textContent = "";
  if (G.mode === "files") {
    ["include", "script", "style", "import", "dom", "class", "api", "href"].forEach((k) => {
      const st = LINK[k];
      if (st) box.append(legendItem(st.color, st.dash, st.label));
    });
  } else {
    box.append(legendItem(CALL_STYLE.same.color, "", "same file"));
    box.append(legendItem(CALL_STYLE.cross.color, CALL_STYLE.cross.dash, "cross file"));
    box.append(legendItem(CALL_STYLE.recur.color, CALL_STYLE.recur.dash, "recursion"));
    box.append(legendItem(LINK.mcall.color, LINK.mcall.dash, "method"));
  }
}

/* ---------- mouse ---------- */
function initView() {
  let drag = null;

  canvasEl.addEventListener("mousedown", (e) => {
    if (closest(e.target, ".inline")) return;            /* let the editor scroll/select */
    const head = closest(e.target, ".card-head");
    if (head && !closest(e.target, "button")) {
      const c = G.cards.get(Number(head.parentElement.dataset.file));
      if (!c) return;
      drag = { kind: "card", c, sx: e.clientX, sy: e.clientY, ox: c.x, oy: c.y, moved: false, down: e.target };
      e.preventDefault();
      return;
    }
    if (!closest(e.target, ".card")) {
      drag = { kind: "pan", sx: e.clientX, sy: e.clientY, ox: G.view.x, oy: G.view.y, moved: false, down: e.target };
      canvasEl.classList.add("panning");
    }
  });

  window.addEventListener("mousemove", (e) => {
    if (!drag) return;
    const dx = e.clientX - drag.sx, dy = e.clientY - drag.sy;
    if (!drag.moved && Math.abs(dx) + Math.abs(dy) > 3) drag.moved = true;
    if (!drag.moved) return;
    if (drag.kind === "card") {
      drag.c.x = drag.ox + dx / G.view.k;
      drag.c.y = drag.oy + dy / G.view.k;
      G.pos[posKey(drag.c.f.path)] = { x: drag.c.x, y: drag.c.y };
      placeCard(drag.c);
      scheduleEdges();
    } else {
      G.view.x = drag.ox + dx;
      G.view.y = drag.oy + dy;
      applyView();
    }
  });

  window.addEventListener("mouseup", (e) => {
    const d = drag;
    drag = null;
    canvasEl.classList.remove("panning");
    if (!d) return;
    if (d.moved) {
      if (d.kind === "card") savePositions();
      return;
    }
    if (d.kind === "pan") {                             /* click on empty canvas */
      clearSelection();
      return;
    }
    /* click (not drag) on the header: edit in place, on the whole card too */
    if (d.c && closest(d.down, ".card-head")) openInline(d.c.f.path);
  });

  canvasEl.addEventListener("wheel", (e) => {
    if (closest(e.target, ".inline")) return;            /* scroll the code, not the map */
    e.preventDefault();
    const r = canvasEl.getBoundingClientRect();
    zoomAt(e.clientX - r.left, e.clientY - r.top, e.deltaY < 0 ? 1.12 : 1 / 1.12);
  }, { passive: false });

  cardsEl.addEventListener("click", (e) => {
    const edit = closest(e.target, "button[data-act='edit']");
    if (edit) {
      const c = G.cards.get(Number(edit.closest(".card").dataset.file));
      if (c) openInline(c.f.path);
      return;
    }
    const row = closest(e.target, ".row");
    if (row) selectSym(Number(row.dataset.sym));
  });

  cardsEl.addEventListener("mouseover", (e) => {
    const card = closest(e.target, ".card");
    if (card) highlightFile(Number(card.dataset.file));
  });
  cardsEl.addEventListener("mouseleave", clearHighlight);
  canvasEl.addEventListener("mouseleave", clearHighlight);

  cardsEl.addEventListener("dblclick", (e) => {
    const row = closest(e.target, ".row");
    if (!row) return;
    const s = G.syms.get(Number(row.dataset.sym));
    if (!s) return;
    const f = G.files.get(s.file);
    if (!f) return;
    G.selSym = s.id;                                  /* keep the links of this row open */
    drawEdges();
    updateHud();
    openInline(f.path, s);
  });

  applyView();
}
