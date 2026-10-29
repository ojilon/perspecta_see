/* inline-editor.js — the text editor that opens *inside* the file's own card.
 *
 * Why inside the node: the map is the codebase, so editing should not throw you
 * into a separate window and lose your place. Expanding the card keeps the
 * file's links, its folder and its neighbours visible around the text.
 *
 * What it does:
 *   - loads the file with /api/file, saves with /api/save (Ctrl+S)
 *   - draws its own line numbers (a <pre> translated by the scroll offset)
 *   - highlights the line range of the function you clicked, so a row in the
 *     card and the code it points at are visibly the same thing
 *   - remembers the scroll position across the rescan that follows a save
 */

const CODE_LH = 18;   /* must match --code-lh in base.css */
const CODE_PAD = 8;   /* textarea padding-top */

function mountInline(c) {
  if (c.inline) return c.inline;

  const box = make("div", "inline");
  const bar = make("div", "inline-bar");
  const title = make("span", "inline-title");
  const dirty = make("span", "dirty", "●");
  const saved = make("span", "saved", "saved");
  const pos = make("span", "inline-pos", "L1:1");
  const saveBtn = make("button", "mini primary", "Save");
  const closeBtn = make("button", "mini", "Close");
  saveBtn.dataset.act = "save";
  closeBtn.dataset.act = "close";
  bar.append(title, dirty, saved, pos, saveBtn, closeBtn);

  const code = make("div", "code");
  const gutter = make("pre", "gutter");
  const scroll = make("div", "scroll");
  const scope = make("div", "scope");
  const ta = make("textarea", "inline-code");
  ta.spellcheck = false;
  ta.wrap = "off";
  ta.setAttribute("autocomplete", "off");
  scroll.append(scope, ta);
  code.append(gutter, scroll);
  box.append(bar, code);
  c.el.append(box);

  const rec = { box, title, dirty, saved, pos, ta, gutter, scope, code, lineCount: 0 };
  c.inline = rec;

  ta.addEventListener("input", () => {
    markDirty(true);
    const lines = (ta.value.match(/\n/g) || []).length + 1;
    if (lines !== rec.lineCount) renderGutter(rec);   /* typing rarely adds a line */
  });
  ta.addEventListener("scroll", () => syncScroll(c));
  ta.addEventListener("keydown", (e) => {
    if (e.key === "Tab") {
      e.preventDefault();
      e.stopPropagation();
      const t = ta.value, a = ta.selectionStart, b = ta.selectionEnd;
      ta.value = t.slice(0, a) + "    " + t.slice(b);
      ta.selectionStart = ta.selectionEnd = a + 4;
      markDirty(true);
    } else if (e.key === "Escape") {
      e.stopPropagation();
      closeInline();
    } else if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "s") {
      e.preventDefault();
      e.stopPropagation();
      saveInline();
    }
  });
  ta.addEventListener("click", () => updateCaret(c));
  ta.addEventListener("keyup", () => updateCaret(c));
  saveBtn.addEventListener("click", (e) => { e.stopPropagation(); saveInline(); });
  closeBtn.addEventListener("click", (e) => { e.stopPropagation(); closeInline(); });
  return rec;
}

function renderGutter(rec) {
  rec.lineCount = rec.ta.value.split("\n").length;
  let out = "";
  for (let i = 1; i <= rec.lineCount; i++) out += i + "\n";
  rec.gutter.textContent = out;
}

function syncScroll(c) {
  const rec = c.inline;
  if (!rec) return;
  rec.gutter.style.transform = "translateY(" + (-rec.ta.scrollTop) + "px)";
  const top = Number(rec.scope.dataset.top || 0);
  rec.scope.style.top = (CODE_PAD + top - rec.ta.scrollTop) + "px";
}

function updateCaret(c) {
  const rec = c.inline;
  if (!rec) return;
  const before = rec.ta.value.slice(0, rec.ta.selectionStart);
  const line = before.split("\n").length;
  const col = before.length - before.lastIndexOf("\n");
  rec.pos.textContent = "L" + line + ":" + col;
}

function scrollToLine(rec, line) {
  rec.ta.scrollTop = Math.max(0, (line - 1) * CODE_LH - 60);
}

/* Highlight the body of `sym` (or clear the highlight). */
function setScope(c, sym) {
  const rec = c.inline;
  if (!rec) return;
  if (!sym) {
    rec.scope.classList.remove("on");
    rec.scope.dataset.top = 0;
    return;
  }
  rec.scope.classList.add("on");
  rec.scope.dataset.top = (sym.line - 1) * CODE_LH;
  rec.scope.style.height = Math.max(CODE_LH, (sym.end - sym.line + 1) * CODE_LH) + "px";
  syncScroll(c);
}

function markDirty(on) {
  E.dirty = !!on;
  if (E.card && E.card.inline) E.card.inline.dirty.classList.toggle("on", !!on);
  markTreeDirty(E.path, !!on);
}

/* The little dot in the tree row of the open file. */
function markTreeDirty(path, dirty) {
  if (!path) return;
  document.querySelectorAll("#tree-body .file").forEach((row) => {
    if (row.dataset.path !== path) return;
    let dot = row.querySelector(".unsaved");
    if (dirty && !dot) {
      dot = make("span", "unsaved", "●");
      row.append(dot);
    } else if (!dirty && dot) {
      dot.remove();
    }
  });
}

/* ---------- open / close / save ---------- */
async function openInline(path, sym, opts) {
  opts = opts || {};
  const file = [...G.files.values()].find((f) => f.path === path);
  if (!file) return;
  const c = G.cards.get(file.id);
  if (!c) return;

  if (E.path === path && c.inline) {          /* already open: just re-aim it */
    E.sym = sym || E.sym;
    setScope(c, sym);
    if (sym) scrollToLine(c.inline, sym.line);
    ensureVisible(c);
    return;
  }
  if (E.dirty && E.path && E.path !== path && !opts.force) {
    if (!confirm("Discard unsaved changes in " + E.path + "?")) return;
  }
  if (E.card && E.card !== c) {
    markTreeDirty(E.path, false);
    collapseInline(E.card);
  }

  const rec = mountInline(c);
  c.el.classList.add("editing");
  E.path = path;
  E.card = c;
  E.sym = sym || null;
  markTreeActive(path);

  /* A 680px editor at "fit" zoom is unreadable: get close enough to work, then
     make sure the whole expanded card is on screen. */
  if (G.view.k < 0.7) focusCard(file.id);
  else ensureVisible(c);

  let text = opts.text;
  if (text === undefined) {
    rec.title.textContent = path + " · loading…";
    try {
      text = await apiReadFile(path);
    } catch (err) {
      setStatus("Cannot open " + path + ": " + err.message, true);
      collapseInline(c);
      return;
    }
    if (E.path !== path) return;              /* another file was opened meanwhile */
  }

  rec.ta.value = text;
  rec.title.textContent = path;
  renderGutter(rec);
  rec.code.style.height = clamp(rec.lineCount * CODE_LH + 2 * CODE_PAD, 150, 520) + "px";
  markDirty(false);
  rec.saved.classList.remove("on");
  updateCaret(c);
  setScope(c, sym);
  if (sym) scrollToLine(rec, sym.line);
  else rec.ta.scrollTop = opts.scroll || 0;
  syncScroll(c);
  if (opts.focus !== false) rec.ta.focus({ preventScroll: true });
  drawEdges();
}

function collapseInline(c) {
  c.el.classList.remove("editing");
  if (c.inline) c.inline.ta.blur();
  if (E.card === c) {
    E.path = null;
    E.card = null;
    E.sym = null;
    E.dirty = false;
    markTreeActive(null);
  }
  drawEdges();
}

function closeInline() {
  const c = E.card;
  if (!c) return;
  if (E.dirty && !confirm("Discard unsaved changes in " + E.path + "?")) return;
  markTreeDirty(E.path, false);
  collapseInline(c);
}

async function saveInline() {
  if (!E.path || !E.card || !E.card.inline) return;
  const rec = E.card.inline;
  try {
    await apiSaveFile(E.path, rec.ta.value);
  } catch (err) {
    setStatus("Save failed: " + err.message, true);
    return;
  }
  markDirty(false);
  rec.saved.classList.add("on");
  setTimeout(() => rec.saved.classList.remove("on"), 1500);
  setStatus("Saved " + E.path + " · re-scanning…");
  await rescan({ reopen: E.path, scroll: rec.ta.scrollTop });
}
