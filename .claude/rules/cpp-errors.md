---
paths:
  - "**/*.hpp"
  - "**/*.cpp"
---

# C++ errors, types and idioms

## Errors
- Non-throwing operations set the thread-local `join::lastError` (`std::error_code`) and return `-1` / `nullptr` / `false`: `lastError = std::error_code (errno, std::generic_category ());` or `lastError = make_error_code (Errc::InvalidParam);`.
- `generic_category` is the default for POSIX errno; the proactor/io_uring code uses `system_category`. Follow the neighbouring file.
- Constructors and one-time setup throw `std::system_error (errno, <category of the neighbouring file>, "<syscall> failed")` (or rethrow `std::system_error (lastError, "...")` after a non-throwing call), documented with `@throw`. Exceptions and `std::function` are to be avoided, never banned outright: use or propose them only with a clear justification, and not on the data path described in `hot-path.md`.
- Capture `errno` right after the failing call.

## Idioms
- Named casts only: `static_cast` (common), `reinterpret_cast` at the OS/wire boundary, rarely `const_cast`; no C casts or `dynamic_cast`. `nullptr`, never `NULL`. `uint8_t`, `size_t` unprefixed.
- Prefer join's own `Function`, `Variant`, `View`, `Mutex` + `ScopedLock`, `Thread`, `Queue`, allocators. Not used here: `std::variant`, `std::mutex`, `std::lock_guard`; `std::span`/`std::optional` only in the data API (SAX handlers, `Value`, readers/writers). Used: `std::string_view`, `std::atomic`, `std::unique_ptr`, `std::shared_ptr` (`Ptr` aliases) and `std::function` (legacy manager listeners, thread entry points) off the hot path; new listeners use `Function`.
- Locks via `ScopedLock`, never manual `lock`/`unlock`. Network byte order via `htons`/`ntohl`...; bit reinterpretation via `std::bit_cast`, host-order tests via `std::endian`.
