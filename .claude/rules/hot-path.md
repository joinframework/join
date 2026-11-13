---
paths:
  - "core/include/**"
  - "core/src/**"
  - "data/include/**"
  - "data/src/**"
---

# Hot paths: performance and determinism first

Targets: zero allocation, zero lock, bounded latency, low jitter. Data path: proactor/reactor submit-complete-wakeup, queues, allocators, timer wheel, statistics, socket I/O, parser/writer loops. Constructors, setup, teardown, configuration and listener management are control path (may allocate, throw, lock). Judge per method, not per file; ask the user if unsure.

## No allocation
- Avoid `new`, `malloc`, `std::string`, growing `std::vector`, `std::function`, `std::shared_ptr`, exceptions: no flat ban, but any of them needs a strong justification stated up front. Memory comes from join allocators (`LocalMem`, bump, slab) sized at construction; value trees use the bump allocator.
- Parser helpers: buffer-based `noexcept` APIs only; no `std::string`/`u16string` overloads, `codecvt`, locale.

## No lock, progress guarantees
- No `Mutex`, condition, `ScopedLock` or blocking syscall (locks stay in setup, `ThreadPool`, `Condition`); use atomics and the lock-free queues.
- Prefer **wait-free**, then **lock-free**. Wait-free: `Spsc::tryPush`/`tryPop` (blocking `push`/`pop` spin with `Backoff`). Lock-free: the slab free list (CAS + `SlabTaggedIndex` generation). `Mpsc`/`Mpmc` are sequence-slot queues, not strictly lock-free (a preempted producer can stall the consumer; `Mpmc` bulk push spins on `_seq`). Document the real guarantee, never a stronger one.
- Weakest topology that fits (SPSC < MPSC < MPMC); one writer per cache line before CAS contention. CAS loops bounded or provably progressing; ABA prevented by a generation tag (`SlabTaggedIndex`; `_seq` is a slot ticket, not ABA protection); document reclamation.
- Explicit memory orders: `acquire`/`release` pairs, `relaxed` for pure counters, `seq_cst` only with a reason. Reason for aarch64; validate with `tsan`.

## False sharing
- Independently written atomics on their own line: `alignas (64)` (`QueueSync`, `BasicStats`, `_wakeupState`). Group fields written by one thread; keep read-mostly fields (`_capacity`) apart.
- Cross-thread slots and commands are `struct alignas (64)`, padded with `(64 - (sizeof (...) % 64)) % 64`. When changing a shared layout, `static_assert` it (`sizeof`/`alignof`/`offsetof`) and state which thread writes which field. Keep the literal `64`.

## Policies and build time
- Variability is a policy parameter, not a runtime flag, virtual call or `std::function`: `IoPolicy`, `SyncPolicy` (`SyncBinding`), `ClockPolicy`; data uses mode parameters (`JsonReadMode`, `JsonWriteMode`). A policy is a stateless struct of `static constexpr` members or tags; optional members detected with `requires` (`has_x_v`), defaulted by an `x_v` specialisation (`io_policy.hpp`); select with `if constexpr` or specialisation. `Function<Sig, Capacity>` is the non-allocating callable.
- Compute at build time: `constexpr`/`consteval` tables (`digitPairs`, `hexDigits`, `pow10`), traits, `static_assert`, sizes and masks derived from template parameters. No function-local statics with dynamic initialization (local `static constexpr` tables are fine). No `strtod`/`std::to_string` on the data path (`data/` uses dconv; `core` does not depend on it). Never recompute a compile-time value in a loop.

## Determinism and proof
- Loops bounded by input or constant; recursion depth-limited. No hidden syscalls, locale or `errno` side effects. `JOIN_UNLIKELY` on error branches, `JOIN_LIKELY` on the expected path, `JOIN_ALWAYS_INLINE` only on measured inner functions.
- Claim a gain only after measuring in `release` (skill `benchmark`); never trade correctness for speed without an equivalence test (`*_roundtrip_test.cpp`, `parse_test.cpp`, stress tests). Report what, how, before/after.
