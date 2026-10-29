/* tree.js — the folder column on the left.
 *
 * Same data as the map, just linear: one row per file, grouped by folder.
 * Folders collapse (the state lives in T.open), and the file open in the inline
 * editor is highlighted, so the two views always agree about "where you are".
 */

const T = { open: new Set(), first: true, root: null };

/* path -> { dirs: Map, files: [] } */
function treeOf(files) {
  const root = { dirs: new Map(), files: [] };
  files.forEach((f) => {
    let node = root;
    const parts = f.path.split("/");
    parts.slice(0, -1).forEach((name, i) => {
      if (!node.dirs.has(name)) {
        node.dirs.set(name, { dirs: new Map(), files: [], path: parts.slice(0, i + 1).join("/") });
      }
      node = node.dirs.get(name);
    });
    node.files.push(f);
  });
  return root;
}

function countFiles(node) {
  let n = node.files.length;
  node.dirs.forEach((d) => { n += countFiles(d); });
  return n;
}

function treeRow(f) {
  const row = make("div", "file");
  row.dataset.lang = f.lang;
  row.dataset.path = f.path;
  row.title = f.path + " — open it inside its node";
  row.append(make("span", "dot"), make("span", "fname", basename(f.path)));
  row.onclick = () => revealFile(f.path);
  return row;
}

function fileRows(node, ul) {
  node.files.sort((a, b) => a.path.localeCompare(b.path)).forEach((f) => {
    const li = make("li");
    li.append(treeRow(f));
    ul.append(li);
  });
}

function dirRows(node, ul) {
  [...node.dirs.entries()].sort((a, b) => a[0].localeCompare(b[0])).forEach(([name, child]) => {
    const li = make("li");
    const head = make("div", "dir");
    const inner = make("ul");
    head.dataset.dir = child.path;

    const draw = () => {
      const open = T.open.has(child.path);
      head.textContent = "";
      head.append(make("span", "caret", "▾"),
                  make("span", "dname", name + "/"),
                  make("span", "count", String(countFiles(child))));
      head.classList.toggle("closed", !open);
      inner.style.display = open ? "" : "none";
    };
    head.onclick = () => {
      if (T.open.has(child.path)) T.open.delete(child.path);
      else T.open.add(child.path);
      draw();
    };

    li.append(head, inner);
    ul.append(li);
    dirRows(child, inner);
    fileRows(child, inner);
    draw();
  });
}

function openAllDirs(node) {
  node.dirs.forEach((d) => { T.open.add(d.path); openAllDirs(d); });
}

function buildTree(data) {
  const box = $("tree-body");
  box.textContent = "";
  const root = treeOf(data.files);
  if (T.root !== G.root) {             /* different project: forget collapse state */
    T.open.clear();
    T.first = true;
    T.root = G.root;
  }
  if (T.first) {                       /* first build of this project: nothing hidden */
    openAllDirs(root);
    T.first = false;
  }
  const ul = make("ul");
  dirRows(root, ul);
  fileRows(root, ul);
  box.append(ul);
  markTreeActive(E.path);
  applyTreeFilter();
}

/* Highlight the file that is open in the inline editor. */
function markTreeActive(path) {
  document.querySelectorAll("#tree-body .file").forEach((row) => {
    row.classList.toggle("active", row.dataset.path === path);
  });
}

/* A file was clicked in the tree: bring its card into view and open it. */
function revealFile(path) {
  const f = [...G.files.values()].find((x) => x.path === path);
  if (!f) return;
  const card = G.cards.get(f.id);
  if (card) { ensureVisible(card); focusCard(f.id); }
  openInline(path);
}

/* Dim the rows that the search box filtered out. */
function applyTreeFilter() {
  const q = (G.filter || "").toLowerCase();
  document.querySelectorAll("#tree-body .file").forEach((row) => {
    row.classList.toggle("dim", !!q && !row.dataset.path.toLowerCase().includes(q));
  });
}
