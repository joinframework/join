---
name: coverage
description: Check join coverage is not degraded versus main (gcc and clang). Use at closing.
---

# Coverage

Coverage must not drop on **gcc and clang**. Use the `debug` presets (only ones with coverage on) and CI's `gcovr` options. `gcc-debug` has io_uring and `clang-debug` does not: compare each compiler only with itself.

```bash
C=gcc    # repeat with C=clang
GCOV=gcov; [ "$C" = clang ] && GCOV='llvm-cov gcov'
cmake --preset $C-debug && cmake --build --preset $C-debug
find build/$C/debug -name '*.gcda' -delete
ctest --preset $C-debug
gcovr --root . --gcov-executable "$GCOV" --exclude-pattern-prefix LCOV --exclude-branches-by-pattern '.*' \
      --exclude '.*/tests/.*' --exclude '.*/samples/.*' --json-summary <scratchpad>/$C-${SIDE:-after}.json --print-summary build/$C/debug
```

Baseline: do the same on `origin/main` in a worktree (`git fetch origin main && git worktree add --detach <scratchpad>/base origin/main`; configure, build, ctest, gcovr, same preset) with `SIDE=before`, into `$C-before.json`. Remove the worktree (`git worktree remove --force <scratchpad>/base`) at the end, also when a step failed.

Compare total line coverage and, above all, every changed file. Any drop fails: add tests (or `LCOV_EXCL_*` for deliberately unreachable code, sparingly) and re-measure.

Needs `gcovr` and `llvm-cov` matching clang, and root for the privileged tests (skipped or failing otherwise, which hides their coverage). When a tool is missing or the change touches privileged code, save the steps above, after `set -e`, to `<scratchpad>/coverage.sh` and run it in CI's image (`docker pull joinframework/join-ci:latest` first if it lacks `gcovr`), on copies so the host `build/` is untouched:

```bash
rsync -a --exclude build ./ <scratchpad>/head/
docker run --rm --privileged --sysctl net.ipv6.conf.all.disable_ipv6=0 -v <scratchpad>:<scratchpad> -w <scratchpad>/head \
       joinframework/join-ci:latest bash -c "bash <scratchpad>/coverage.sh; rc=\$?; chown -R $(id -u):$(id -g) <scratchpad>; exit \$rc"
```

Same for the baseline with `-e SIDE=before -w <scratchpad>/base`. `--privileged` is root-equivalent (skill `test`). If neither works, report coverage as **not verified**, never "not degraded".
