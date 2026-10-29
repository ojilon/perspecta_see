/* editor.js: a plain text editor panel. Good enough to change the project from inside itself. */

const E = { path: null, dirty: false };
const edBox = $("editor"), edText = $("editor-text"), edPath = $("editor-path");

function refreshEditorTitle() {
  edPath.textContent = (E.path || "") + (E.dirty ? "  •" : "");
}

async function openFile(path) {
  if (E.dirty && path !== E.path && !confirm("Discard unsaved changes in " + E.path + "?")) return;
  try {
    edText.value = await apiReadFile(path);
  } catch (err) {
    setStatus("Cannot open " + path + ": " + err.message, true);
    return;
  }
  E.path = path;
  E.dirty = false;
  edBox.hidden = false;
  refreshEditorTitle();
}

async function saveFile() {
  if (!E.path) return;
  try {
    await apiSaveFile(E.path, edText.value);
    E.dirty = false;
    refreshEditorTitle();
    await rescan();                                       /* re-parse so the graph matches the new code */
    setStatus("Saved " + E.path);
  } catch (err) {
    setStatus("Save failed: " + err.message, true);
  }
}

function closeEditor() {
  if (E.dirty && !confirm("Discard unsaved changes?")) return;
  E.path = null;
  E.dirty = false;
  edBox.hidden = true;
}

function initEditor() {
  edText.addEventListener("input", () => { E.dirty = true; refreshEditorTitle(); });
  edText.addEventListener("keydown", (e) => {
    if (e.key === "Tab") {                                /* keep focus in the editor, insert spaces */
      e.preventDefault();
      edText.setRangeText("    ", edText.selectionStart, edText.selectionEnd, "end");
      E.dirty = true;
      refreshEditorTitle();
    }
  });
  document.addEventListener("keydown", (e) => {
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "s" && E.path) {
      e.preventDefault();
      saveFile();
    }
  });
  $("save").onclick = saveFile;
  $("close").onclick = closeEditor;
}