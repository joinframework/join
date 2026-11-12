---
paths:
  - "**/tests/**"
---

# Tests

- GoogleTest/GMock. **One file per class**, snake_case + `_test.cpp` (`TcpAcceptor` -> `tcp_acceptor_test.cpp`); template policy variants follow the existing split (`local_bump_test.cpp`, `heap_bump_test.cpp`).
- **One test per public method**, named after it: `TEST (TcpAcceptor, accept)`. Constructors and special members: `create` (`construct` if the class has a `create` method), `move`, `copy`. Suite is the class name; with `TEST_F`, a fixture named `<Class>` or `<Class>Test` (both exist: copy the neighbouring file). All paths of a method (success, errors, boundaries, overloads) live in its single test. Extra tests for non-method concerns (`scopedLock`, benchmarks, `inheritConstruct`) are allowed as separate `TEST`s. A new, removed or renamed public method changes its test in the same change.
- Each test has a three-line `/**` / ` * @brief Test <method> method.` / ` */` block (the dominant form in the existing tests). Prefer `ASSERT_*`; `EXPECT_*` only when later lines stay meaningful. Assert moved-from and boundary states explicitly.
- Register in the module `tests/CMakeLists.txt`, like the neighbours:

```cmake
add_executable(<name>.gtest <name>_test.cpp)
target_link_libraries(<name>.gtest ${JOIN_<MODULE>} GTest::gtest_main)
add_test(NAME <name>.gtest COMMAND <name>.gtest)
install(TARGETS <name>.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
```

- New code paths need tests (coverage is tracked). Tests are deterministic under TSan/ASan: prefer futures/conditions over sleeps (some existing tests sleep; do not add more); no shared global state.
- Tests needing root, veth or netns use a fixture that probes the privilege and calls `GTEST_SKIP` when it is missing (e.g. `PingTest`; `NdpLinkTest` predates the rule and does not skip).
- Never weaken or delete an assertion to make a test pass.
