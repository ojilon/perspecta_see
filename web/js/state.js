/* state.js — the small amount of shared state, and the colour language.
 *
 * G holds the graph (last scan + where things are on screen),
 * E holds the one inline editor that may be open,
 * LINK says what every reference kind looks like.
 *
 * The kind strings are produced by C (include/model.h) and must stay in sync
 * with the switch in src/json.c. Unknown kinds simply fall back to grey, so a
 * new kind added on the C side never breaks the page.
 */

const LANG = {
  html: { label: "HTML", col: 0 },
  css:  { label: "CSS",  col: 1 },
  js:   { label: "JS",   col: 2 },
  c:    { label: "C",    col: 3 },
  h:    { label: "H",    col: 3 },
  other:{ label: "?",    col: 4 },
};

/* color + dash ("" = solid) + what to call it in the legend */
const LINK = {
  include: { color: "#5fb3ff", dash: "",    label: "include" },
  script:  { color: "#ff9f6b", dash: "",    label: "script" },
  style:   { color: "#b794ff", dash: "5 4", label: "stylesheet" },
  import:  { color: "#8f9bff", dash: "5 4", label: "import" },
  class:   { color: "#d6a3ff", dash: "2 4", label: "css class" },
  dom:     { color: "#ffd666", dash: "2 4", label: "dom id" },
  api:     { color: "#4be3a0", dash: "",    label: "api route" },
  href:    { color: "#ffc078", dash: "2 4", label: "page link" },
  call:    { color: "#3aa0ff", dash: "",    label: "call" },
  mcall:   { color: "#6fd0d6", dash: "4 3", label: "method call" },
};
const LINK_FALLBACK = { color: "#8aa2c8", dash: "3 3", label: "link" };

function styleOf(kind) { return LINK[kind] || LINK_FALLBACK; }

/* call links are coloured by what they connect, not by their kind */
const CALL_STYLE = {
  same:  { color: "#3aa0ff", dash: "",    label: "same file" },
  cross: { color: "#c78bff", dash: "6 4", label: "cross file" },
  recur: { color: "#4be3a0", dash: "3 3", label: "recursion" },
};

const G = {
  data: null,              /* the last /api/scan JSON */
  files: new Map(),        /* file id  -> file object */
  syms: new Map(),         /* sym id   -> symbol object */
  cards: new Map(),        /* file id  -> card record (see cards.js) */
  view: { x: 40, y: 40, k: 1 },
  selSym: null,            /* selected symbol id, or null */
  mode: "files",           /* "files" | "calls" */
  filter: "",              /* search box text */
  root: "",                /* scanned root, also the localStorage key */
  pos: {},                 /* "root::path" -> {x,y} dragged card positions */
};

const E = {
  path: null,              /* file open in the inline editor */
  card: null,              /* its card record */
  sym: null,               /* function we jumped to, if any */
  dirty: false,
  scroll: 0,               /* kept across a rescan so saving does not jump */
};

/* ---------- remembering dragged cards ---------- */
const POS_KEY = "perspecta.pos.v1";

function loadPositions() {
  try {
    G.pos = JSON.parse(localStorage.getItem(POS_KEY) || "{}") || {};
  } catch (err) {
    G.pos = {};
  }
}

function savePositions() {
  try {
    localStorage.setItem(POS_KEY, JSON.stringify(G.pos));
  } catch (err) { /* private mode: positions just will not survive a reload */ }
}
