# math

Part of [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries), a collection of small, reusable C++ libraries.

[![CI](https://github.com/Kronuz/math/actions/workflows/ci.yml/badge.svg)](https://github.com/Kronuz/math/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)

Header-only integer-math helpers for C++20: a non-negative modulo, and unsigned add and subtract that saturate on overflow.
Each helper is a few lines of standard C++ with no state and no allocation. Reach
for it when a ring index has to wrap a negative offset, or a counter has to stop
at its limit instead of wrapping around.

## ✨ Highlights

- **`modulus(-1, 3) == 2`**, where C++'s `%` gives `-1`. A negative modulus
  throws `std::invalid_argument`.
- **`add` and `sub`** on unsigned types set an overflow flag and saturate to the
  type's maximum or minimum.
- **`min<T>` and `max<T>`** give the range an accuracy list covers: twice its
  last entry, negated for `min`, clamped to `long`, `long long` or their
  unsigned forms; `double` gets its numeric limits.
- **One header, no dependencies.**

## 🚀 Quick start

```cpp
#include "math.hh"

#include <cstdio>

int main() {
	std::printf("%d\n", modulus(-7, 3));         // 2 (C++'s -7 % 3 is -1)

	bool overflow = false;                       // add() sets it, never clears it
	unsigned sum = add(0xFFFFFFFFu, 1u, overflow);
	std::printf("%u %d\n", sum, overflow);       // 4294967295 1
}
```

## 📦 Installation

Requires a C++20 compiler and CMake 3.16 or newer; no dependencies. CI builds it
on Linux (x86_64, arm64), macOS (arm64, x86_64), and FreeBSD, plus ASan, UBSan,
and TSan builds.

```cmake
include(FetchContent)
FetchContent_Declare(math
  GIT_REPOSITORY https://github.com/Kronuz/math.git
  GIT_TAG        2730483144785dc73892e2274f37ad380a600729)  # or main for the tip
FetchContent_MakeAvailable(math)

target_link_libraries(my_app PRIVATE math::math)
```

```cpp
#include "math.hh"
```

## 🎯 When to use it

Use it to fold negative offsets into a ring and to keep unsigned counters from
wrapping silently.

Not for:

- **Signed overflow checks.** `add` and `sub` test `x + y < x`, which detects
  wrap-around for unsigned types only; on signed types the overflow is undefined
  behavior.
- **A zero modulus.** Only a negative modulus throws; `modulus(v, 0)` divides by
  zero.
- **A flag that resets itself.** `add` and `sub` set `overflow` on wrap-around
  and leave it alone otherwise, so start it at `false`.
- **The lowest `double`.** `min<double>` returns
  `std::numeric_limits<double>::min()`, the smallest positive normal value.

## 📚 Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md): each helper's contract.
- [AGENTS.md](AGENTS.md): repository map and traps.
- [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#math): the catalog entry.

## 🛠️ Building and testing

```sh
cmake -B build
cmake --build build
ctest --test-dir build
```

## 🤝 Contributing

Issues and pull requests are welcome. Read [AGENTS.md](AGENTS.md) and the
catalog's [rules of engagement](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#rules-of-engagement)
first; CI must pass on every platform.

## 📄 License

MIT, Copyright (c) 2026 Germán Méndez Bravo and Copyright (c) 2015-2020 Dubalu
LLC. See [LICENSE](LICENSE). Extracted from [Xapiand](https://github.com/Kronuz/Xapiand).
