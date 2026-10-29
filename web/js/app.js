/* app.js: start-up and wiring. Reads like a table of contents for the other files. */

function renderTree(data) {
  const root = { dirs: Object.create(null), files: [] };
  data.files.forEach((f) => {
    let n = root;
    f.path.split("/").slice(0, -1).forEach((p) => {
      n = n.dirs[p] || (n.dirs[p] = { dirs: Object.create(null), files: [] });
    });
    n.files.push(f);
  });
  const build = (node) => {
    const ul = make("ul");
    Object.keys(node.dirs).sort().forEach((name) => {
      const li = make("li");
      li.append(make("div", "dir", name + "/"), build(node.dirs[name]));
      ul.append(li);
    });
    node.files.forEach((f) => {
      const li = make("li", "file", f.path.split("/").pop());
      li.onclick = () => { focusCard(f.id); openFile(f.path); };
      ul.append(li);
    });
    return ul;
  };
  const t = $("tree");
  t.textContent = "";
  t.append(build(root));
}

async function rescan() {
  const root = $("root").value.trim() || ".";
  setStatus("Scanning…");
  const d = await apiScan(root);
  $("root").value = d.root;
  buildGraph(d);
  renderTree(d);
  const s = d.stats;
  setStatus(s.files + " files · " + s.syms + " symbols · " + s.edges + " links · " + s.ms + " ms" +
            (d.unresolved.length ? " · " + d.unresolved.length + " unresolved (see G.data.unresolved in the console)" : ""));
}

async function scanClicked() {
  try {
    await rescan();
    fitView();
  } catch (err) {
    setStatus(err.message, true);
  }
}

async function boot() {
  initGraph();
  initEditor();
  $("scan").onclick = scanClicked;
  $("fit").onclick = fitView;
  $("root").addEventListener("keydown", (e) => { if (e.key === "Enter") scanClicked(); });
  try {
    const p = await apiPing();
    $("root").value = p.root || p.cwd;                    /* first run: scan the folder the server started in */
    await rescan();
    fitView();
  } catch (err) {
    setStatus("Cannot reach the C server: " + err.message, true);
  }
}

boot();