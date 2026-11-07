# Releases and versioning

## Versioning

Semantic versioning, starting at `0.x`. While at `0.x`, minor bumps may break
anything (CLI flags, json layout, error codes). That is fine — the tool is a
hobby project in active development.

| Version | Meaning for perspecta_see |
| ------- | -------------------------- |
| 0.1.0   | First usable release: `scan`, `index`, `tokenize`; txt and json output; `--version`. |
| 0.2.0   | `report`, `check`, `gen` commands; ASCII views; diagnostics. Breaking changes allowed. |
| 1.0.0   | Stable CLI and json schema; documented commands; breaking changes require a major bump. |

## Embedding the version

A `--version` flag prints the version, e.g. `perspecta_see 0.1.0`.

Simple way without extra tooling: keep a `VERSION` file at the repo root
containing `0.1.0`, and generate `src/version.h` from it in the build script:

```bat
:: build.bat (Windows)
@echo off
set /p VER=<VERSION
echo #define PERSPECTA_VERSION "%VER%" > src\version.h
gcc -std=c11 -Wall -Wextra -o perspecta.exe src\main.c src\error.c src\file.c src\version.c
```

`src/version.c` simply prints it:

```c
#include <stdio.h>
#include "version.h"

void print_version(void) {
    printf("perspecta_see %s\n", PERSPECTA_VERSION);
}
```

`main.c` checks `--version` first and calls `print_version()`. The generated
`version.h` should be gitignored (add `src/version.h` to `.gitignore`).

On Termux the same idea with `sh`:

```sh
# build.sh
VER=$(cat VERSION)
echo "#define PERSPECTA_VERSION \"$VER\"" > src/version.h
gcc -std=c11 -Wall -Wextra -o perspecta src/main.c src/error.c src/file.c src/version.c
```

## Build flags

Development (warnings-heavy):

```
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0
```

Release:

```
gcc -std=c11 -Wall -Wextra -O2 -DNDEBUG
```

On Windows you may also need `-D_CRT_SECURE_NO_WARNINGS` for the CRT functions.
Consider `-Wconversion -Wshadow -Wformat=2` once the codebase is stable
(they are noisy early on — add them gradually).

Debug vs release script: keep two scripts, `build.bat`/`build.sh` (debug, fast
to compile) and `build-release.bat`/`build-release.sh` (release, `-O2`).

## JSON schema stability

The json output carries a schema version field:

```json
{
  "schema": 1,
  "files": [ ... ],
  "summary": { ... }
}
```

Rule: whenever the json format changes (field added, removed, or retyped), bump
the `schema` integer. Consumers of json can check the field and adapt. Do not
change the format without bumping `schema`.

## Release checklist

- [ ] Builds clean with warnings on (`-Wall -Wextra -Wpedantic`)
- [ ] Test programs pass
- [ ] `CHANGELOG.md` updated for the version
- [ ] `VERSION` file updated; `src/version.h` regenerated
- [ ] Git tag created (`v0.1.0`, `v0.2.0`, ...)
- [ ] Short release notes written (what ships, what is explicitly not supported)
