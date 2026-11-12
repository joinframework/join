---
name: compile
description: Configure and build join with the CMake presets, as CI does. Use to compile a change or fix build errors.
---

# Compile

Dev loop vs closing: see `AGENTS.md` "Definition of done". Dev loop: `gcc-debug` only, one target. Closing: gcc and clang, `debug` + `release` + `tsan` + `asan`; announce it, it is slow.

```bash
cmake --preset <preset>
cmake --build --preset <preset> -j [--target <name>.gtest]
```

Presets: `gcc-` / `clang-` + `debug`, `release`, `tsan`, `asan`; trees in `build/<compiler>/<kind>`. Fix warnings (`-Werror`), never silence them. `JOIN_ENABLE_IO_URING` differs per preset (see `CMakePresets.json`), so a path can be compiled by one compiler and not the other.

## What CI builds on every PR

1. `clang-format` at the version pinned in `build.yml` over the tree (gates everything; `exclude-regex: 'build'` also skips paths containing `build`, e.g. `value_builder*`, so format those locally). Check `clang-format --version`: if it differs, ask the user to install it; a format diff seen only in CI points to the version gap.
2. gcc and clang x amd64 and arm64, in `joinframework/join-ci`: `release` then `debug`; `tsan` and `asan` on amd64.
3. CodeQL on `gcc-debug`.

## Report

Presets built, with the first error or warning in full on failure. Missing dependencies: see the README, and ask the user to install them.
