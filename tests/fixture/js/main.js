// js/main.js — the JS side of the fixture.
// Comments mention "/api/ghost" and getElementById("ghost") on purpose:
// the scans run on a comment-masked copy, so neither may become a link.
function boot() {
  const el = document.getElementById("app");
  el.textContent = inlineHello();
  fetch("/api/ghost");   /* not defined by any C file: must be dangling */
  fetch("/api/real");    /* defined in c/server.c: must resolve */
  return el;
}
