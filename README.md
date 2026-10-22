# math

Part of [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries), a collection of small, reusable C++ libraries.

[![CI](https://github.com/Kronuz/math/actions/workflows/ci.yml/badge.svg)](https://github.com/Kronuz/math/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)

Small integer-math helpers for C++20: a modulo that is always non-negative, and
add and subtract that detect unsigned overflow and saturate.

## ✨ Highlights

- **`modulus(-1, 3) == 2`**, where C++'s `%` gives `-1`. A non-positive modulus
  throws.
- **`add` and `sub`** on unsigned types set an overflow flag and saturate to the
  type's maximum or minimum.
- **`min<T>` and `max<T>`** for a bucketed accuracy list.
- **One header, no dependencies.**

## 📦 Installation

Requires a C++20 compiler; CMake 3.14 or newer for the CMake route. CI builds it
on Linux (x86_64, arm64), macOS (arm64, x86_64), and FreeBSD, plus ASan, UBSan,
and TSan builds.

```cmake
include(FetchContent)
FetchContent_Declare(math
  GIT_REPOSITORY https://github.com/Kronuz/math.git
  GIT_TAG        main)
FetchContent_MakeAvailable(math)

target_link_libraries(my_app PRIVATE math::math)
```

## 🚀 Quick start

```cpp
#include "math.hh"

#include <cassert>

int main() {
	assert(modulus(-7, 3) == 2);

	bool overflow = false;
	auto sum = add(0xFFFFFFFFu, 1u, overflow);   // saturates
	assert(overflow && sum == 0xFFFFFFFFu);
}
```

## 🎯 When to use it

- ✅ Wrapping indexes into rings, and counters that must not wrap silently.
- ❌ Signed overflow checks: `add` and `sub` are for unsigned types.

## 📚 Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md): each helper's contract.
- [AGENTS.md](AGENTS.md): repository map and traps.

## 🛠️ Building and testing

```sh
cmake -B build && cmake --build build && ctest --test-dir build
```

## 🤝 Contributing

Issues and pull requests are welcome. Read [AGENTS.md](AGENTS.md) first. CI must
pass on every platform.

## 📄 License

MIT, Copyright (c) 2015-2019 Dubalu LLC. See [LICENSE](LICENSE). Extracted from
[Xapiand](https://github.com/Kronuz/Xapiand).
