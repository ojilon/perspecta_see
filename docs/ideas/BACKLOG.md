# Backlog — parked ideas

Put every idea here that is not part of the current phase. If it distracts you
from today's session, write it down here and continue. Review the backlog only
at phase boundaries or during buffer weeks.

## Parked (not this year)

- Full C preprocessor (macro expansion, `#if`, defined())
- Full C type resolution (typedef names, struct member types)
- Language server / IDE integration (VS Code extension)
- GUI or web frontend (the old browser demo — archived, see below)
- Incremental indexing (don't re-scan unchanged files)
- Parallel scanning (threads) — likely too heavy for a Celeron anyway
- Binary analysis (symbols, DWARF)
- Windows-specific features (file locking, UTF-16 paths) beyond what gcc needs
- Plugin system
- Config file format beyond simple key=value
- Language support beyond C (C++, Rust, Odin) — parse only C for now

## Maybe for year 2

Decide during Phase 12. Write phase ideas here once the year-2 plan exists.

## Archived: old browser demo

The original browser-based visualizer is moved out of the way before the first
PR (tag or archive branch first, so nothing is lost). It stays out of the C
repo unless someone deliberately brings it back.

## Rule

Ideas go here, not into today's session. If you want to try something, use a
throwaway `exp/` branch, not the current phase.
