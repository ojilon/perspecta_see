/* api.js: the only place that talks to the C server. Each function = one request. */

const $ = (id) => document.getElementById(id);

function setStatus(text, isError) {
  const s = $("status");
  s.textContent = text;
  s.className = isError ? "error" : "";
}

async function apiPing() {
  const r = await fetch("/api/ping");
  return r.json();
}

async function apiScan(root) {
  const r = await fetch("/api/scan?root=" + encodeURIComponent(root));
  if (!r.ok) throw new Error(await r.text());
  return r.json();
}

async function apiReadFile(path) {
  const r = await fetch("/api/file?path=" + encodeURIComponent(path));
  if (!r.ok) throw new Error(await r.text());
  return r.text();
}

async function apiSaveFile(path, text) {
  const r = await fetch("/api/save?path=" + encodeURIComponent(path), { method: "POST", body: text });
  if (!r.ok) throw new Error(await r.text());
  return r.json();
}