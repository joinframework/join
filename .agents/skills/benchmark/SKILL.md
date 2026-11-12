---
name: benchmark
description: Check join performance versus main with json-benchmark, pack-benchmark and queue-benchmark. Use at closing when benchmarked code changed.
---

# Benchmark a change

Required at closing when the change touches benchmarked code: `data/` parsers and writers (`json-benchmark` for JSON, `pack-benchmark` for MessagePack `pack.hpp`, both for shared code such as `value*`, `sax*`), `core/` queues and memory backends (`queue.hpp`, `memory.hpp`: `queue-benchmark`). Bump/slab allocators have no benchmark: report **not verified** or measure ad hoc. Done means no degradation versus `main` beyond the run-to-run spread. If no suitable machine is available, report the item as **not verified** (indicative WSL2 numbers may be given but do not satisfy it) and ask the user how to proceed.

## 1. Ask first

Before running anything, ask the user: **is a real machine available, ideally both aarch64 and x86_64?** Memory ordering and SIMD behave differently on each, and numbers from a hypervisor are not trustworthy: `queue-benchmark` pins threads (`pthread_setaffinity_np`), which cannot control physical core placement under WSL2, and the `run_bench.sh` of `json-benchmark` and `pack-benchmark` refuse to start unless the governor is `performance` and boost is off, then wait for the CPU to settle, `ALLOW_NOISY=1` overriding it. If only WSL2 is available, say that results are indicative only, relative, and not publishable. The repos are expected next to join (`../<repo>`); if one is missing, clone `https://github.com/joinframework/<repo>.git` there, never into the join tree.

## 1b. Put the machine in performance mode

Read the current state first: `cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor` and `cat /sys/devices/system/cpu/cpufreq/boost` (Intel with `intel_pstate` uses `/sys/devices/system/cpu/intel_pstate/no_turbo` instead; not verified here). Expected: governor `performance`, boost off (`0`; `no_turbo` = `1`). Note the original values.

```bash
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
echo 0 | sudo tee /sys/devices/system/cpu/cpufreq/boost              # AMD / acpi-cpufreq
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo    # Intel intel_pstate
```

Run the governor command and the boost command for this CPU with `sudo` only when justified, and say why. If `sudo` cannot run here (password prompt, no rights), give them exactly and ask the user to run them with the `! <command>` prefix so they execute in this session. Do not use `ALLOW_NOISY=1` to skip the check unless the user agrees to noisy numbers. When the campaign is over, remind the user of the original values so they can restore them. If the machine has no cpufreq interface (WSL2, some VMs), say the mode cannot be set and the numbers are indicative only.

## 2. Compare, same machine, same session

The repos fetch join from `main` by default (`FetchContent`, `GIT_TAG main`). To measure the branch, point them at the local checkout (`FETCHCONTENT_SOURCE_DIR_JOIN`, honoured by all three):

```bash
cmake -S . -B <scratchpad>/<repo>-branch -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_SOURCE_DIR_JOIN=<path-to-join>
```

Check each tree measures the intended join: `grep FETCHCONTENT_SOURCE_DIR_JOIN <build>/CMakeCache.txt` must show its path, otherwise the numbers are GitHub `main`. json-benchmark and pack-benchmark pin a dconv tag: if the change needs an unreleased dconv, also pass `-DFETCHCONTENT_SOURCE_DIR_DCONV=<path-to-dconv>` to both trees.

Baseline: the same with a join `origin/main` worktree (`git -C <path-to-join> fetch origin main && git -C <path-to-join> worktree add --detach <scratchpad>/base origin/main`) in `<scratchpad>/<repo>-main`. Build both, then run the **binaries of each tree directly**, baseline and branch back to back, several times, alternating, with results written to the scratchpad. Do not use `run_bench.sh` for the comparison: each rebuilds `build/` against GitHub `main` and regenerates the committed `plots/`. Leave no changes in the benchmark repos; remove the worktree at the end.

- **json-benchmark** (its `run_bench.sh`: conformance, then 3 repetitions, min reported, `taskset -c 2`, `GLIBC_TUNABLES=glibc.malloc.mmap_threshold=33554432:glibc.malloc.trim_threshold=4294967296`): `<build>/benchmarks/performance/bench_<parse|stringify|prettify|statistics>_join` with the same `taskset` and `GLIBC_TUNABLES`, keep the minimum; datasets `canada`, `citm_catalog`, `twitter`. Also run `<build>/benchmarks/conformance/conformance --output <scratchpad>/<main|branch>-conformance.json`: a conformance drop is a regression. `sonic-cpp` is built unconditionally with `-march=native`; building on aarch64 is not verified.
- **pack-benchmark** (same `run_bench.sh` protocol as json-benchmark): `<build>/benchmarks/performance/bench_<parse|serialize|statistics>_join`, same `taskset`, `GLIBC_TUNABLES` and conformance; datasets `canada`, `citm_catalog`, `twitter` (`.msgpack`).
- **queue-benchmark** (its `run_bench.sh`: 10 repetitions, means): e.g. `<build>/benchmarks/mpmc/bench_mpmc_throughput --benchmark_filter="Join.*4/4/" --benchmark_repetitions=10 --benchmark_display_aggregates_only=true --benchmark_out=<scratchpad>/<main|branch>-mpmc.json`. Throughput, bulk and latency for `Spsc`, `Mpsc`, `Mpmc`, `LocalMem` and `ShmMem`.

Only the join entries matter for a regression check; the other libraries are context.

## 3. Report

Give: machine (CPU model, architecture, governor/boost, kernel, native or WSL2), repetitions, and a before/after table for the affected benchmarks with the relative change. Call a change real only if it exceeds the run-to-run spread; otherwise say it is within noise. When a delta is close to the spread, rerun with 5 repetitions per side and decide with google benchmark's `compare.py` (`<build>/_deps/googlebenchmark-src/tools/compare.py benchmarks <main>.json <branch>.json`, needs `scipy`; merge each side's runs into one JSON first): with 3 per side its Mann-Whitney test cannot reach p < 0.05. A mechanism read from the diff (fewer atomics, false sharing removed) is a hypothesis until `perf stat -e cycles,instructions,cache-misses,branch-misses` shows it (needs `kernel.perf_event_paranoid` <= 1). Never claim a gain you did not measure, and say plainly when the machine made the numbers unreliable.
