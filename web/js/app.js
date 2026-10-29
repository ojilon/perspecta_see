/* app.js — start-up and wiring. Reads like a table of contents for the rest.
 *
 * boot()        ping the server, scan, draw
 * rescan()      the one place that talks to /api/scan; keeps the open editor
 * render()      scan JSON -> maps, cards, tree, hud
 * selectSym()   one function: show its links, dim the rest
 * applyFilter() the search box
 * updateHud()   the little status panel in the corner
 */

const HUD = { showDangling: false, clippedCalls: false };

/* ---------- scan ---------- */
async function rescan(opts) {
  opts = opts || {};
  const root = ($("root").value || "").trim() || ".";
  const reopen = opts.reopen || E.path;
  const scroll = opts.scroll !== undefined ? opts.scroll : 0;
  setStatus("Scanning " + root + " …");
  let data;
  try {
    data = await apiScan(root);
  } catch (err) {
    setStatus(err.message, true);
    throw err;
  }
  G.root = data.root;
  try { localStorage.setItem("perspecta.root", data.root); } catch (err) { /* ignore */ }
  $("root").value = data.root;

  /* the cards are about to be rebuilt: let go of the old editor, then reopen */
  E.path = null;
  E.card = null;
  E.sym = null;
  E.dirty = false;
  render(data);
  if (reopen && [...G.files.values()].some((f) => f.path === reopen)) {
    await openInline(reopen, null, { scroll, force: true, focus: false });
  }
  setStatus(summary(data));
}

function summary(d) {
  const s = d.stats;
  const bits = [
    plural(s.files, "file"),
    plural(s.syms, "symbol"),
    plural(s.edges, "link"),
    plural(s.calls, "call"),
    s.ms + " ms",
  ];
  if (s.unresolved) bits.push(plural(s.unresolved, "dangling link"));
  return bits.join("  ·  ");
}

function render(data) {
  G.data = data;
  G.files = new Map(data.files.map((f) => [f.id, f]));
  G.syms = new Map(data.syms.map((s) => [s.id, s]));
  G.selSym = null;
  HUD.clippedCalls = data.calls.length > MAX_CALL_LINES;
  buildCards(data);
  buildTree(data);
  applyFilter();
  buildLegend();
  drawEdges();
  updateHud();
}

/* ---------- selection ---------- */
function selectSym(id) {
  G.selSym = G.selSym === id ? null : id;
  drawEdges();
  updateHud();
}

function clearSelection() {
  if (G.selSym === null) return;
  G.selSym = null;
  drawEdges();
  updateHud();
}

/* ---------- search ---------- */
function applyFilter() {
  G.filter = ($("search").value || "").trim();
  const q = G.filter.toLowerCase();
  let hits = 0;
  G.cards.forEach((c) => {
    const hit = !q || c.f.path.toLowerCase().includes(q) ||
      c.syms.some((s) => s.name.toLowerCase().includes(q));
    c.el.classList.toggle("filtered", !hit);
    if (hit) hits++;
  });
  applyTreeFilter();
  updateHud(hits);
}

function firstMatch() {
  const q = G.filter.toLowerCase();
  if (!q) return null;
  return [...G.cards.values()].find((c) =>
    c.f.path.toLowerCase().includes(q) || c.syms.some((s) => s.name.toLowerCase().includes(q))) || null;
}

/* ---------- hud ---------- */
function pill(text, cls, onclick) {
  const p = make("span", "pill" + (cls ? " " + cls : ""), text);
  if (onclick) p.onclick = onclick;
  return p;
}

function updateHud(hits) {
  const hud = $("hud");
  hud.textContent = "";
  const d = G.data;
  if (!d) return;
  const row = make("div", "hud-row");

  if (G.selSym !== null) {
    const s = G.syms.get(G.selSym);
    const file = G.files.get(s.file);
    const out = d.calls.filter((l) => l.from === s.id).length;
    const inn = d.calls.filter((l) => l.to === s.id).length;
    const title = make("b", "", s.kind === "route" ? s.name : s.name + "()");
    row.append(title);
    row.append(pill(file ? file.path : "?", "mono"));
    if (s.end > s.line) row.append(pill("lines " + s.line + "–" + s.end, "mono"));
    row.append(pill("uses " + out));
    row.append(pill("used by " + inn));
    const open = make("button", "mini", "Open code");
    open.onclick = () => { if (file) openInline(file.path, s); };
    const clear = make("button", "mini ghost", "Clear");
    clear.onclick = clearSelection;
    row.append(open, clear);
  } else {
    const st = d.stats;
    row.append(pill(plural(st.files, "file")));
    row.append(pill(plural(st.syms, "symbol")));
    row.append(pill(plural(st.edges, "link")));
    row.append(pill(plural(st.calls, "call")));
    row.append(pill(st.ms + " ms"));
    if (st.unresolved) {
      row.append(pill(plural(st.unresolved, "dangling link"), "warn", () => {
        HUD.showDangling = !HUD.showDangling;
        updateHud();
      }));
    }
    if (typeof hits === "number" && G.filter) row.append(pill(hits + (hits === 1 ? " match" : " matches"), "hit"));
    if (HUD.clippedCalls && G.mode === "calls") row.append(pill("showing first " + MAX_CALL_LINES + " calls"));
  }
  hud.append(row);

  if (HUD.showDangling && G.selSym === null && d.unresolved.length) {
    const list = make("div", "dangling");
    d.unresolved.forEach((u) => {
      const file = G.files.get(u.file);
      const line = make("div", "", (file ? file.path : "?") + ":" + u.line + "  →  " + u.target);
      line.title = "click to open this line";
      line.addEventListener("click", async () => {
        if (!file) return;
        await openInline(file.path, null);
        if (E.card && E.card.inline) {
          scrollToLine(E.card.inline, u.line);
          syncScroll(E.card);
        }
      });
      const k = make("span", "k", u.kind + " ");
      line.prepend(k);
      list.append(line);
    });
    hud.append(list);
  }
}

/* ---------- top bar + keyboard ---------- */
function setMode(mode) {
  G.mode = mode;
  $("mode-files").classList.toggle("on", mode === "files");
  $("mode-calls").classList.toggle("on", mode === "calls");
  buildLegend();
  drawEdges();
  updateHud();
}

function wireTopbar() {
  $("scan").onclick = async () => {
    if (E.dirty && !confirm("Re-scanning reloads the open file and drops unsaved changes. Continue?")) return;
    try {
      await rescan();
      fitView();
    } catch (err) { /* rescan already reported it */ }
  };
  $("root").addEventListener("keydown", (e) => {
    if (e.key === "Enter") { e.preventDefault(); $("scan").click(); }
  });
  $("fit").onclick = fitView;
  $("tidy").onclick = tidyLayout;
  $("zoom-in").onclick = () => zoomBy(1.2);
  $("zoom-out").onclick = () => zoomBy(1 / 1.2);
  $("mode-files").onclick = () => setMode("files");
  $("mode-calls").onclick = () => setMode("calls");

  const search = $("search");
  search.addEventListener("input", debounce(applyFilter, 120));
  search.addEventListener("keydown", (e) => {
    if (e.key === "Enter") {
      const c = firstMatch();
      if (c) { ensureVisible(c); focusCard(c.f.id); }
    } else if (e.key === "Escape") {
      search.value = "";
      search.blur();
      applyFilter();
    }
  });
}

function wireKeys() {
  document.addEventListener("keydown", (e) => {
    const typing = /^(INPUT|TEXTAREA)$/.test(e.target.tagName);
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "s") {
      if (E.path) { e.preventDefault(); saveInline(); }
      return;
    }
    if (typing) return;
    if (e.key === "/") { e.preventDefault(); $("search").focus(); }
    else if (e.key === "Escape") {
      if (G.selSym !== null) clearSelection();
      else if (E.path) closeInline();
    } else if (e.key === "f") fitView();
    else if (e.key === "+" || e.key === "=") zoomBy(1.2);
    else if (e.key === "-") zoomBy(1 / 1.2);
    else if (e.key === "t") tidyLayout();
  });
}

/* ---------- go ---------- */
async function boot() {
  loadPositions();
  initView();
  buildLegend();
  wireTopbar();
  wireKeys();
  try {
    const p = await apiPing();
    const remembered = typeof lastRoot === "function" ? lastRoot() : "";
    $("root").value = remembered || p.root || p.cwd;
    await rescan();
    fitView();
  } catch (err) {
    setStatus("Cannot reach the C server: " + err.message, true);
  }
}

boot();
