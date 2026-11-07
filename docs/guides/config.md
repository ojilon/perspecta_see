# Configuration in perspecta_see

Configuration grows in three stages. Start with stage 1; add the next only when
the current one hurts.

## Stage 1: CLI flags (now)

`argc`/`argv` only. Flags like `--format txt|json`, `--output file`,
`--version`. Zero files to manage; works on Windows and Termux identically.

Good for: flags you set once per command.

Bad for: flags you repeat on every command (typing `--output report.json`
every time gets old).

## Stage 2: a simple key=value config file

A file named `perspecta.cfg` next to the project (or in the user's home
directory), parsed by hand — no library:

```
# comment
format = json
output = report.json
```

Parser sketch: read lines, skip blank lines and lines starting with `#`, split
on the first `=`, trim spaces around key and value. Store in a small array of
`{key, value}` pairs; CLI flags override config-file values.

Good for: per-project defaults.
Bad for: anything that changes per-command.

## Stage 3: maybe more later

Environment variables (`PERSPECTA_FORMAT`) only if stage 2 is not enough. Do
not add until a real need appears.

## Reasoning

- Start with zero configuration: a new user runs `perspecta scan src` and gets
  output immediately.
- Add a config file only when repetition becomes the error you keep making.
- Parse it yourself: a 20-line parser teaches you file reading and string
  handling, and needs no dependencies.
- CLI flags always win over the config file — explicit beats implicit.
