#!/usr/bin/env python3
"""End-to-end assertions for the scanner and the HTTP API.

Run through tests/run.sh (or `make test`), which starts the server, copies the
fixture to a scratch folder and passes both here as argv[1] = port,
argv[2] = the scratch project folder.

Each block below is one promise the parser makes. They are deliberately small:
when a heuristic changes, one of these should fail loudly instead of the map
quietly growing a phantom node.
"""
import json
import sys
import urllib.parse
import urllib.request

PORT = sys.argv[1]
ROOT = sys.argv[2]
BASE = f"http://127.0.0.1:{PORT}"

failures = []


def check(name, ok, detail=""):
    print(("PASS  " if ok else "FAIL  ") + name + ("" if ok else "   " + str(detail)))
    if not ok:
        failures.append(name)


def get(path):
    with urllib.request.urlopen(BASE + path) as r:
        return r.read()


def scan(root):
    return json.loads(get("/api/scan?root=" + urllib.parse.quote(root)))


data = scan(ROOT)
files = {f["id"]: f["path"] for f in data["files"]}
syms = data["syms"]

# ---- 1. comments are not code -------------------------------------------------
routes = sorted(s["name"] for s in syms if s["kind"] == "route")
check("only real routes are found", routes == ["/api/real"], routes)

ghost_ids = [s["name"] for s in syms if s["kind"] == "id" and s["name"] == "ghost"]
check("getElementById() inside a comment is not a DOM id", ghost_ids == [], ghost_ids)

unres = {(files[u["file"]], u["kind"], u["target"]) for u in data["unresolved"]}
check("fetch('/api/ghost') is reported as dangling",
      ("js/main.js", "api", "/api/ghost") in unres, unres)
check("fetch('/api/real') is NOT dangling",
      all(u[2] != "/api/real" for u in unres), unres)

# ---- 2. $("id") / getElementById() produce DOM links --------------------------
edges = {(files[e["from"]], files[e["to"]], e["kind"], e["n"]) for e in data["edges"]}
check("js -> html dom link exists",
      any(e[0] == "js/main.js" and e[1] == "index.html" and e[2] == "dom" for e in edges),
      sorted(edges))
check("js -> c api link exists",
      ("js/main.js", "c/server.c", "api", 1) in edges, sorted(edges))

# ---- 3. inline <script> / <style> are parsed ---------------------------------
inline_fns = [(files[s["file"]], s["name"], s["line"])
              for s in syms if s["kind"] == "func" and files[s["file"]] == "index.html"]
check("inline <script> defines a function", inline_fns == [("index.html", "inlineHello", 20)],
      inline_fns)

inline_classes = [(files[s["file"]], s["name"])
                  for s in syms if s["kind"] == "class" and files[s["file"]] == "index.html"]
check("inline <style> defines a class", ("index.html", "card") in inline_classes,
      inline_classes)

# ---- 4. scopes and symbol links ---------------------------------------------
boot = next((s for s in syms if s["name"] == "boot"), None)
check("function scope runs 4..10", boot is not None and (boot["line"], boot["end"]) == (4, 10), boot)

calls = {(data["syms"][c["from"]]["name"], data["syms"][c["to"]]["name"], c["kind"])
         for c in data["calls"] if c["from"] >= 0}
check("js function calls an inline <script> function",
      ("boot", "inlineHello", "call") in calls, sorted(calls))
check("js function uses the DOM", ("boot", "app", "dom") in calls, sorted(calls))

# ---- 5. the API: read, save, rescan -----------------------------------------
text = get("/api/file?path=js/main.js").decode()
check("GET /api/file returns the file", "function boot()" in text, len(text))

edited = text.replace("function boot()", "function boot() // touched by tests")
req = urllib.request.Request(
    BASE + "/api/save?path=" + urllib.parse.quote("js/main.js"),
    data=edited.encode(), method="POST")
with urllib.request.urlopen(req) as r:
    saved = json.loads(r.read())
check("POST /api/save reports the byte count",
      saved.get("ok") is True and saved["bytes"] == len(edited.encode()), saved)

with open(ROOT + "/js/main.js") as f:
    check("the saved text reached the disk", f.read() == edited)

again = scan(ROOT)
check("a rescan still parses the edited file",
      any(s["name"] == "boot" for s in again["syms"]))

# ---- 6. the path guard -------------------------------------------------------
def status(path, data=None, method="GET"):
    req = urllib.request.Request(BASE + path, data=data, method=method)
    try:
        with urllib.request.urlopen(req) as r:
            return r.status
    except urllib.error.HTTPError as e:
        return e.code


for bad in ["/api/file?path=../../etc/passwd",
            "/api/file?path=/etc/passwd",
            "/api/file?path=README.md"]:
    check("rejects " + bad.split("=", 1)[1], status(bad) == 400, status(bad))
check("rejects a save outside the root",
      status("/api/save?path=../escape.js", b"x", "POST") == 400)

print()
if failures:
    print(str(len(failures)) + " check(s) failed")
    sys.exit(1)
print("all checks passed")
