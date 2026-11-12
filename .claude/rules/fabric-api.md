---
paths:
  - "fabric/**"
  - "core/include/join/reactor.hpp"
  - "core/include/join/proactor*.hpp"
---

# fabric / event-driven API conventions

- New protocol and socket leaf classes are `final` (Ndp, Dhcp clients/servers); legacy `Arp` and `Ping` are not. No user-overridable virtual callbacks.
- Listeners are mutated through `reactor.invoke`: single-slot `setXxxListener (cb)` / `unsetXxxListener ()` returning `int` (`Ndp`, `Dhcp`), or `uint64_t addXxxListener (cb)` / `void removeXxxListener (id)` (`NeighborManager`, `RouteManager`, `InterfaceManager`). Single-slot callbacks are `join::Function` (`Ndp`, `Dhcp`); the legacy managers use `std::function`. New listeners use `Function`.
- The final class registers with the reactor through the base's register helper once fully constructed, not in the base constructor (`BasicNdp`, `BasicDhcp`).
- Sync / async shape: `op ()` never waits; `op (Result&, timeout)` waits and fills the reference. The sync form fails with `std::errc::resource_deadlock_would_occur` (EDEADLK) when `_reactor.isReactorThread ()`.
- Keep the `Protocol` template parameter for consistency with `Dhcp`, even if only one protocol exists.
- Common behaviour lives in the base class (e.g. neighbor discovery in `BasicNdp`), role-specific code in the final classes.
- The lib is moving from the reactor to the proactor (NDP first): do not add new reactor-only code paths without checking with the user.
- Every feature exists on both epoll and io_uring, or its scope is reduced explicitly.
- Caller-owned buffers and endpoints are never copied; they stay valid until the handler runs. Signatures forbid dangling temporaries (`asyncWriteTo` takes a non-const `Endpoint&`, `connect` a `const Endpoint&`).
