---
paths:
  - "**/*.hpp"
  - "**/*.cpp"
---

# C++ style

`.clang-format` is enforced (hook, or `clang-format` at the CI version); never hand-format. Errors and idioms: `cpp-errors.md`.

## Layout
- C++20, no extensions. Space before every `(` (`foo (a)`, `TEST (S, n)`; except `operator~(T)`). Allman braces on every control statement, no one-line functions or lambdas. Constructor initializers one per line, leading comma. Digit separators `1'000'000`.
- Header: MIT license, `#ifndef JOIN_<MODULE>_<NAME>_HPP` (no `#pragma once`; the `*_uring*`/`*_epoll*` backend headers have none and are quote-included by their front header), content in indented `namespace join`. Backends guarded by `JOIN_HAS_IO_URING` / `_NUMA` / `_XDP`.
- Include groups, in order: `// dconv.` (data only), `// libjoin.`, `// Libraries.`, `// C++.`, `// C.` (kernel `<linux/...>` too); `<join/...>` for join headers.
- Templates are implemented in class in the header (only the proactor / io_ring_buffer backends define out of class). Other classes go in `src/<name>.cpp` (`using join::X;` after the includes), except small or hot-path classes kept in the header (`Backoff`, clocks, `PackWriter`): follow the neighbours. Helpers nest in the owning class; helper namespaces are `details`.
- Each out-of-class definition (`.cpp`, proactor backend headers) has this banner:

```cpp
// =========================================================================
//   CLASS     : Mutex
//   METHOD    : lock
// =========================================================================
```

## Naming
- `PascalCase` types, enum classes and values, template parameters, aliases; `camelCase` functions, accessors `get`/`set`/`is`/`has`.
- Members `_camelCase`, default-initialised when they have a sentinel (`int _handle = -1;`), no `m_`. `this->` only for base members (dependent bases in templates, `std::streambuf` in the stream buffers). Public constants `camelCase`, private `static constexpr` `_camelCase`; traits `snake_case_v`; macros `JOIN_UPPER`.
- Bitmask enums: `enum class X : uint32_t`, values `1L << n`, full set of free bit operators.

## Classes
- Order: `public`, `protected`, `private`; constructors, special members, destructor, API.
- Special members explicit (`= delete` copy then move, or `= default`); moved-from objects stay valid and empty. `explicit` single-argument constructors, `override` always, `virtual ~X` only on real bases, `final` on new leaves (legacy `Arp`, `Ping` are not).
- `noexcept` when it cannot throw, `constexpr` wherever possible, `[[maybe_unused]]` on unused parameters, `static_assert` with message. No `[[nodiscard]]`.

## Doxygen
- Every header declaration: `/**`, ` * @brief ...` (lowercase), then `@param`/`@return`/`@throw` lines, ` */`. One short line per tag, a noun phrase ending with a period; no rationale (it goes in the reply or commit). Wording: `create instance.`, `destroy instance.`, `copy constructor.`, `move assignment operator.`, `@param other other object to copy.`, `@return current object.`. Class members: `/// text.` above; enum values and plain struct fields: trailing `/**< ... */`.
- No comments inside method bodies except `// LCOV_EXCL_LINE` and `LCOV_EXCL_START`/`STOP`.
