# join

[![Test Status](https://github.com/joinframework/join/actions/workflows/test.yml/badge.svg?branch=main)](https://github.com/joinframework/join/actions?query=workflow%3Atest+branch%3Amain)
[![Security Status](https://github.com/joinframework/join/actions/workflows/security.yml/badge.svg?branch=main)](https://github.com/joinframework/join/actions?query=workflow%3Asecurity+branch%3Amain)
[![Codacy](https://app.codacy.com/project/badge/Grade/c2eda80c815e43748d10b9bde0be7087)](https://app.codacy.com/gh/joinframework/join/dashboard)
[![Codecov](https://codecov.io/gh/joinframework/join/branch/main/graph/badge.svg)](https://codecov.io/gh/joinframework/join)
[![Doxygen](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://joinframework.github.io/join/index.html)
[![Release](https://img.shields.io/github/release/joinframework/join.svg)](https://github.com/joinframework/join/releases/latest)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**join** is a **modular C++ network runtime framework for Linux**, designed for
**low-latency**, **high-throughput**, and **system-level networking**.

It provides a set of composable libraries covering networking primitives,
concurrency, serialization, cryptography, and Linux network fabric management.

---

## Design Goals

- Linux-native networking (sockets, netlink, raw sockets)
- Event-driven and reactor-based architecture
- Predictable latency and high throughput
- Strong separation of concerns via modular libraries
- High test coverage and correctness-first design
- Suitable for infrastructure, control-plane, and runtime components

---

## Architecture Overview

join is split into independent libraries that can be built and linked separately.

### `join_core` (mandatory)

The foundation of the framework. All other modules depend on `join_core`.

Provides:
- Socket abstractions (TCP, UDP, Unix, raw, TLS)
- TLS support via OpenSSL
- Event reactor
- Threading primitives and thread pools
- Mutexes, conditions, semaphores
- Timers (monotonic and real-time)
- Lock-free queues (SPSC, MPSC, MPMC)
- IP and MAC address utilities
- Endpoint and protocol helpers
- Core utilities and type traits

### `join_crypto` (optional)

Cryptographic utilities built on top of OpenSSL:
- Base64 encoding/decoding
- Digest and HMAC
- Signatures
- TLS key helpers

### `join_data` (optional)

Data formats and streaming:
- High-performance JSON parser and writer
- SAX-style parsing API
- MessagePack support
- Zero-copy views and buffers
- Streaming compression (zlib)

### `join_fabric` (optional)

Linux network fabric management:
- Network interface abstraction
- Netlink-based interface manager
- ARP client
- Address and name resolution
- Interface state monitoring

### `join_services` (optional)

Application-level protocols:
- HTTP client and server
- Chunked transfer encoding
- SMTP client
- Mail message handling

---

## Dependencies

```bash
sudo apt install libssl-dev zlib1g-dev libgtest-dev libgmock-dev
```

> OpenSSL is required by `join-core` as TLS support is part of the core runtime.

---

## Build Configuration

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DJOIN_ENABLE_CRYPTO=ON \
  -DJOIN_ENABLE_DATA=ON \
  -DJOIN_ENABLE_FABRIC=ON \
  -DJOIN_ENABLE_SERVICES=ON \
  -DJOIN_ENABLE_TESTS=ON
```

---

## Build

```bash
cmake --build build
```

---

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

---

## Documentation

API documentation is generated using **Doxygen** and published via GitHub Pages:

https://joinframework.github.io/join/

---

## Project Scope

join focuses on providing **robust, efficient building blocks** for:
- network runtimes
- system services
- control planes
- high-performance servers
- infrastructure tooling

---

## License

join is licensed under the **MIT License**.
