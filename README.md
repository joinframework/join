# join

[![Coverage Status](https://github.com/joinframework/join/workflows/coverage/badge.svg)](https://github.com/joinframework/join/actions?query=workflow%3Acoverage)
[![Security Status](https://github.com/joinframework/join/actions/workflows/security.yml/badge.svg)](https://github.com/joinframework/join/security/code-scanning)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/c2eda80c815e43748d10b9bde0be7087)](https://app.codacy.com/gh/joinframework/join/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/c2eda80c815e43748d10b9bde0be7087)](https://app.codacy.com/gh/joinframework/join/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Codecov](https://codecov.io/gh/joinframework/join/branch/main/graph/badge.svg)](https://codecov.io/gh/joinframework/join)
[![Coveralls](https://coveralls.io/repos/github/joinframework/join/badge.svg?branch=main)](https://coveralls.io/github/joinframework/join?branch=main)
[![Doxygen](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://joinframework.github.io/join/index.html)
[![GitHub Releases](https://img.shields.io/github/release/joinframework/join.svg)](https://github.com/joinframework/join/releases/latest)
[![GitHub License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/joinframework/join/blob/main/LICENSE)

**join** is a lightweight C++14 network framework library

## Dependencies

To install join framework dependencies do this:
```bash
sudo apt update && sudo apt install libssl-dev zlib1g-dev libgtest-dev libgmock-dev
```

## Download

To download the latest source do this:
```bash
git clone https://github.com/joinframework/join.git
```

## Configuration

To configure **join** with test and coverage enabled do this:
```bash
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DJOIN_ENABLE_TESTS=ON -DJOIN_ENABLE_COVERAGE=ON
```

## Build

To build **join** do this:
```bash
cmake --build build --config Debug
```

## Tests

To test **join** do this:
```bash
ctest --test-dir build --output-on-failure -C Debug
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
