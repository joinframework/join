---
name: test
description: Run join tests (ctest or one gtest), including sanitizers and root-only tests. Use to validate a change or reproduce a failure.
---

# Test

Build first with skill `compile`. Dev loop vs closing: see `AGENTS.md` "Definition of done". At closing, the `coverage` skill follows, and `benchmark` when the change touches benchmarked code.

```bash
ctest --preset <preset> --output-on-failure [-R '^<name>\.gtest$']
build/<compiler>/<kind>/<module>/tests/<name>.gtest --gtest_filter='Suite.test' --gtest_repeat=50
```

`tsan` for races, atomics, reactor/proactor; `asan` for memory, UB, lifetimes. Repeat flaky or concurrent tests under `tsan` before calling them fixed. The `tsan`/`asan` test presets set `TSAN_OPTIONS`/`ASAN_OPTIONS`/`UBSAN_OPTIONS` (`CMakePresets.json`): export the same values when running a binary directly.

## What CI tests on every PR

gcc and clang x amd64 and arm64, in `joinframework/join-ci` (`--privileged`, IPv6): `release` then `debug` ctest; on amd64 `gcovr` coverage, then `tsan` and `asan`. Coverage is generated for gcc and clang, but only the gcc report is published (`deploy-coverage`, which can fail the run).

Privileged tests (`CAP_NET_RAW`, veth, netns) run for real in CI; without root older tests fail locally and newer ones skip (new privileged tests use a fixture that probes the privilege and calls `GTEST_SKIP`, e.g. `PingTest`): ask the user to run them as root, see `AGENTS.md`. A skip is not a pass. `JOIN_ENABLE_IO_URING` differs per preset, so a path can be exercised by one compiler and not the other.

## Run in the CI image (Docker)

On a machine with Docker, the privileged tests can run in the CI image `joinframework/join-ci:latest` (same flags as CI: `--privileged`, IPv6 enabled). `--privileged` is root-equivalent: run it yourself only when justified, otherwise give the command to the user with the `! ` prefix; either way, analyze the log. The repo is copied into the container so the host `build/` is untouched. Example (not verified; change the preset as needed):

```bash
docker run --rm --privileged --sysctl net.ipv6.conf.all.disable_ipv6=0 -v "$PWD":/src:ro joinframework/join-ci:latest bash -c 'mkdir /work && tar --exclude=./build -C /src -cf - . | tar -C /work -xf - && cd /work && git config --global --add safe.directory /work && cmake --preset gcc-debug && cmake --build --preset gcc-debug && ctest --preset gcc-debug --output-on-failure' 2>&1 | tee <scratchpad>/gcc-debug.log
```

Run the same steps for each preset required at closing. Without Docker, ask the user to run the root tests directly (see `AGENTS.md`).

## Report

Presets run and pass/fail counts. Report environmental failures or skips with the output; never claim success you did not observe.
