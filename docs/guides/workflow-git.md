# Git workflow and PR plan

## Branches

- `main` — the stable, buildable line. Only merged PRs land here.
- Feature branches — one per small feature, named `feature/<short-name>`.
- Throwaway branches — experiments that may never be merged, named `exp/<name>`.
- The current branch `refactor/restart_` holds the rewrite. Keep working there
  until the first PR.

## The first PR (one PR, at a stable point)

Before any feature PRs, open **one** first PR to clear out the old browser demo
and establish the new layout. Checklist:

- [ ] New layout builds with a single command (`build.bat` / `build.sh`)
- [ ] CLI prints path, size, and line count using the `File` API (`perspecta.exe src\main.c`)
- [ ] `main.c` no longer uses raw `fopen`
- [ ] Docs exist: `docs/journey/README.md`, phase files, session template, guides
- [ ] Old browser demo code removed **or** moved clearly out of the way:
  - create a tag or archive branch first (e.g. `git branch archive/browser-demo`)
  - then delete the demo files from the working branch
  - nothing is lost: the tag/branch keeps the old code
- [ ] CHANGELOG.md has an Unreleased section

Open this PR against `main`, review it yourself (or with a friend), merge it.
After it is merged, the rewrite is the project.

## After the first PR

- One branch per small feature.
- PRs small enough to review in 10 minutes (one concept, one file change at most where possible).
- Merge at the end of each phase at least — do not let a whole phase sit unmerged.
- Delete feature branches after merging.

## Commit messages

Short imperative subject, one line of why when needed:

```
Add File_read_whole to the file API

Read an entire file into a heap buffer so the scanner can
load source files without byte-by-byte fgetc loops.
```

Subject line: imperative mood ("Add", "Fix", "Remove", "Use"), 50–72 chars.
Body: one paragraph of why, wrapped at 72 chars.

## When NOT to open a PR

- Half-finished experiments (the code does not compile end to end).
- Anything that breaks the current phase's definition of done.
- Spikes to try an idea — use a throwaway `exp/` branch instead; delete it
  when done, or merge only the parts that survive.

## Experiment workflow

1. `git checkout -b exp/try-arena-strings`
2. Try the idea. It may not compile; that is fine on an exp branch.
3. Keep what works, throw the rest away, delete the branch.
4. Port the good parts to a real `feature/` branch and PR it.
