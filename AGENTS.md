# AGENTS.md

**join**: modular C++20 networking runtime for Linux. Modules `core`, `crypto`, `data`, `fabric`, `services` (each `include/join/`, `src/`, `tests/`, optional `samples/`); dependencies `services` -> `fabric` -> `crypto` -> `core`, `services` -> `data` -> `core`.

## Build and test

Presets `gcc|clang` x `debug|release|tsan|asan`, trees in `build/<compiler>/<kind>`. `-Werror` is on.

```bash
cmake --preset gcc-debug && cmake --build --preset gcc-debug && ctest --preset gcc-debug --output-on-failure
```

## Agent files

Skills in `.agents/skills/<name>/SKILL.md`: compile, test, coverage, benchmark, add-gtest, create-pr, release; `<scratchpad>` in them is a temporary directory outside the repository. Run `clang-format` on changed C++ files at the version pinned in `.github/workflows/build.yml`.

Changes to `AGENTS.md` and `.agents/` keep them tool-agnostic and short; `.claude/` is Claude Code only.

## Definition of done

- **Dev loop**: `gcc-debug`, the relevant test only, once the batch is reviewed (not after each edit); no clang, `release`, sanitizers or coverage. Renames: diff and `clang-format` only. Design still moving: code and benchmarks only.
- **Closing** (once, when the user says ready to merge), gcc and clang: scope limited to the request; tests added; `debug`, `release`, `tsan`, `asan` pass; `coverage` not degraded; `benchmark` not degraded if benchmarked code changed (`data/`, `core/` queues and memory backends); `README.md` updated if affected, else say none needed.

Report presets run, results, coverage before/after and README status; never call a partial pass done. A missing tool or wrong version: ask before installing (`sudo` if justified), else give the command with the `! ` prefix, and report the item **not verified**. Root-only tests and system settings (the user is a sudoer): `sudo` if justified with output to a log, or the CI image (skill `test`), or ask the user to run them with `! `; then analyze the log.

## Working agreements

- English for code, comments, commits, PRs, docs and branches.
- Brief answers. Change only what was requested; extras as separate proposals.
- Off the data path, plain code first, measured; tricks, fast paths or new helpers only as a measured, proposed option.
- Show one diff at a time; edit only after an explicit yes covering that diff ("ok" after an either/or question is not a choice). Reading, building, measuring and scratchpad work need no approval.
- Prefer join's primitives (`Mutex`, `Condition`, `Semaphore`, `Queue`, `ThreadPool`, `Function`, `Variant`, `View`, allocators, clocks (`Monotonic`, `Rdtsc`...) and their `Timer`...) over `std::` or new dependencies; check `core/include/join/` first.
- Samples stay minimal: few lines, one `try/catch`.
- Ask before any state-changing git command (except `<scratchpad>` worktrees for measurements) and each PR (skill `create-pr`); read-only ones are fine. Never force-push, push to `main` or merge.
- Never edit `build/` or the generated `core/include/join/version.hpp`.
