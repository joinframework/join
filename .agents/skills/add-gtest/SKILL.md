---
name: add-gtest
description: Add and register a gtest file for a join class. Use when creating tests for new or untested code.
---

# Add a gtest

Input: module and class.

1. Read the header and a neighbouring test (e.g. `core/tests/tcp_acceptor_test.cpp`) for the exact layout and license header.
2. List the class's public methods; create `<module>/tests/<class_snake>_test.cpp` with one `TEST (<Class>, <method>)` each (plus `create` (`construct` if the class has a `create` method)/`move`/`copy`), mimicking the neighbouring file for fixture naming and the `@brief Test <method> method.` form, each covering success, errors, boundaries and overloads. Concurrent code gets a multi-threaded section for TSan.
3. Register it in `<module>/tests/CMakeLists.txt` next to its neighbours:

   ```cmake
   add_executable(<name>.gtest <name>_test.cpp)
   target_link_libraries(<name>.gtest ${JOIN_<MODULE>} GTest::gtest_main)
   add_test(NAME <name>.gtest COMMAND <name>.gtest)
   install(TARGETS <name>.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
   ```
4. Build and run it in dev-loop mode (`gcc-debug`, that target only) with skills `compile` and `test`.

If a test reveals a bug, report the failing assertion and propose a diff first; do not change the code under test unprompted.
