/* api.js — the only file that talks to the C server.
 *
 * One function per endpoint (see include/api.h). Everything is relative to the
 * page, so the same code works when the server is on 127.0.0.1 and when it is
 * behind a proxy. Errors are turned into thrown Error(text) so callers can show
 * the server's own message instead of "something went wrong".
 */

async function apiPing() {
  const r = await fetch("/api/ping");
  if (!r.ok) throw new Error(await r.text());
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
  const r = await fetch("/api/save?path=" + encodeURIComponent(path), {
    method: "POST",
    body: text,
  });
  if (!r.ok) throw new Error(await r.text());
  return r.json();
}
