# join
### High-Performance Modular Networking for the Linux Ecosystem

[![Test Status](https://github.com/joinframework/join/actions/workflows/cd.yml/badge.svg?branch=main)](https://github.com/joinframework/join/actions?query=workflow%3Acd+branch%3Amain)
[![Security Status](https://github.com/joinframework/join/actions/workflows/audit.yml/badge.svg?branch=main)](https://github.com/joinframework/join/actions?query=workflow%3Aaudit+branch%3Amain)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/c2eda80c815e43748d10b9bde0be7087)](https://app.codacy.com/gh/joinframework/join/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/c2eda80c815e43748d10b9bde0be7087)](https://app.codacy.com/gh/joinframework/join/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Codecov](https://codecov.io/gh/joinframework/join/branch/main/graph/badge.svg)](https://codecov.io/gh/joinframework/join)
[![Coveralls](https://coveralls.io/repos/github/joinframework/join/badge.svg?branch=main)](https://coveralls.io/github/joinframework/join?branch=main)
[![Doxygen](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://joinframework.github.io/join/index.html)
[![GitHub Releases](https://img.shields.io/github/release/joinframework/join.svg)](https://github.com/joinframework/join/releases/latest)
[![GitHub License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/joinframework/join/blob/main/LICENSE)

**join** is a **modular C++ network runtime framework for Linux**, designed for
optimized **throughput** and **latency** system-level networking.

It provides a set of composable libraries covering networking primitives,
concurrency, serialization, cryptography, and Linux network fabric management.

---

## 🚀 Design Goals

- Linux-native networking (sockets, netlink, raw sockets)
- Event-driven and reactor-based architecture
- Low-jitter event processing
- Strong separation of concerns via modular libraries
- High test coverage and correctness-first design
- Suitable for infrastructure, control-plane, and runtime components

---

## 🎯 Target Use Cases

**Designed for:**
- Network services and microservices
- Control plane and infrastructure components
- System-level networking tools
- High-performance servers (web, RPC, messaging)

**Not designed for:**
- Sub-microsecond latency requirements (HFT, market data)
- Kernel-bypass networking (use DPDK or RDMA instead)
- Data plane packet processing at 100Gbps

---

## ✨ Why join?

join focuses on providing **robust, efficient building blocks** for:
- network runtimes
- system services
- control planes
- high-performance servers
- infrastructure tooling

---

## 🏗 Modular Architecture

The framework is a collection of specialized modules that build upon one another:

| Module | Purpose | Highlights |
| :--- | :--- | :--- |
| **`core`** | **Foundation** | Epoll Reactor, TCP/UDP/TLS, Unix Sockets, Thread Pools, Lock-Free Queues & Allocator. |
| **`fabric`**| **Network Control** | Netlink Interface Manager, ARP client, DNS Resolver. |
| **`crypto`**| **Security** | OpenSSL Wrappers, HMAC, Digital Signatures, Base64. |
| **`data`** | **Serialization** | High-perf JSON (DOM/SAX), MessagePack, Zlib Streams. |
| **`services`**| **Protocols** | HTTP/1.1 (Client/Server), SMTP, Mail Parsing. |

---

## 🛠️ Build & Integration

### Prerequisites
Install build tools and compilers:
```bash
sudo apt install gcc g++ clang clang-tools libclang-rt-dev cmake ninja-build gdb-multiarch
```

Install required libraries and test dependencies:
```bash
sudo apt install libssl-dev zlib1g-dev libgtest-dev libgmock-dev
```

> **Compilers:** Both GCC and Clang are supported. Clang requires `libclang-rt-dev` for coverage instrumentation (`--coverage`).  
> **OpenSSL** provides the core TLS runtime.

### Optional Dependencies

| Option | Library | Default | Description |
| :--- | :--- | :---: | :--- |
| `JOIN_ENABLE_NUMA` | `libnuma-dev` | `OFF` | Enables NUMA aware memory binding for `LocalMem` and `ShmMem`. |

Install as needed:
```bash
sudo apt install libnuma-dev    # for JOIN_ENABLE_NUMA
```

### Build from Source
```bash
git clone https://github.com/joinframework/join.git
cd join
cmake -B build -DCMAKE_BUILD_TYPE=Release -DJOIN_ENABLE_TESTS=ON
cmake --build build
```

With optional backends:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release \
    -DJOIN_ENABLE_NUMA=ON \
    -DJOIN_ENABLE_TESTS=ON
cmake --build build
```

### Build Options

| Option | Default | Description |
| :--- | :---: | :--- |
| `BUILD_SHARED_LIBS` | `ON` | Build as shared libraries. |
| `JOIN_ENABLE_CRYPTO` | `ON` | Build the crypto module. |
| `JOIN_ENABLE_DATA` | `ON` | Build the data module. |
| `JOIN_ENABLE_FABRIC` | `ON` | Build the fabric module. |
| `JOIN_ENABLE_SERVICES` | `ON` | Build the services module (requires crypto, data, fabric). |
| `JOIN_ENABLE_NUMA` | `OFF` | Enable NUMA support (requires `libnuma-dev`). |
| `JOIN_ENABLE_SAMPLES` | `OFF` | Build sample programs. |
| `JOIN_ENABLE_TESTS` | `OFF` | Build the test suite. |
| `JOIN_ENABLE_COVERAGE` | `OFF` | Enable code coverage instrumentation (requires Debug build). |

### Run Tests
```bash
ctest --test-dir build --output-on-failure
```

---

## 📦 Integration

**join** exports standard CMake targets. To use it in your project:

```cmake
find_package(join REQUIRED)

target_link_libraries(your_app PRIVATE 
    join::core
    join::crypto
    join::data
    join::fabric
    join::services
)
```

---

## 📊 Quality & Performance

Every commit is validated against an extensive test suite to ensure stability in concurrent environments:
* **1000+ Unit Tests** covering networking, concurrency, and data parsing.
* **Security:** Continuous scanning via Codacy and GitHub Security workflows.

---

## 📖 Documentation
* **API Reference:** [Explore the Doxygen Docs](https://joinframework.github.io/join/index.html)
* **License:** Licensed under the [MIT License](LICENSE).
