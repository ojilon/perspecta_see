/* util.js — tiny helpers that every other script uses.
 *
 * No framework, no build step: plain globals loaded in the order written in
 * index.html. Keeping the helpers here means the other files read as features
 * rather than as DOM boilerplate.
 */

const $ = (id) => document.getElementById(id);
const SVGNS = "http://www.w3.org/2000/svg";

function make(tag, cls, text) {
  const el = document.createElement(tag);
  if (cls) el.className = cls;
  if (text !== undefined) el.textContent = text;
  return el;
}

function svgEl(tag, attrs) {
  const el = document.createElementNS(SVGNS, tag);
  for (const k in attrs) el.setAttribute(k, attrs[k]);
  return el;
}

function setStatus(text, isError) {
  const s = $("status");
  s.textContent = text;
  s.classList.toggle("error", !!isError);
}

function basename(path) {
  const i = path.lastIndexOf("/");
  return i < 0 ? path : path.slice(i + 1);
}

function dirname(path) {
  const i = path.lastIndexOf("/");
  return i < 0 ? "" : path.slice(0, i);
}

function clamp(v, lo, hi) { return v < lo ? lo : v > hi ? hi : v; }

/* element.closest(), but safe when the event target is the window/document. */
function closest(target, sel) {
  return target && typeof target.closest === "function" ? target.closest(sel) : null;
}

function plural(n, one, many) {
  return n + " " + (n === 1 ? one : many || one + "s");
}

/* Run `fn` at most once per `ms` of quiet. */
function debounce(fn, ms) {
  let t = 0;
  return (...args) => {
    clearTimeout(t);
    t = setTimeout(() => fn(...args), ms);
  };
}

/* A file id -> "web/js/app.js"-style label, used in tooltips and the hud. */
function filePath(id) {
  const f = G.files.get(id);
  return f ? f.path : "?";
}
