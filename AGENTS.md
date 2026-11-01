# AGENTS

Small integer-math helpers extracted verbatim from Xapiand: `modulus` (always
non-negative), overflow-detecting `add`/`sub`, and `min`/`max<T>(accuracy)`.

- Header-only, standard-library only. Self-contained includes (`<cstdint>`,
  `<limits>`, `<vector>`, `<type_traits>`, `<stdexcept>`) -- the extraction added the
  first three, which Xapiand had been supplying transitively.
- `add`/`sub` overflow detection (`x + y < x`, formed in `T`) is a wrap-around test valid for
  UNSIGNED types of every width (signed overflow is UB). Do not use them on signed types.
- `modulus` computes on unsigned values (the magnitude of a negative `val`), never negating
  `val`: `INT_MIN` has no positive counterpart. `math_ubsan` runs the test under UBSan.
- `modulus` throws `std::invalid_argument` on a non-positive modulus, zero included.
- `add`/`sub` never clear the overflow flag (it is sticky).
- `min<double>` is `numeric_limits<double>::lowest()`.
- `test/test.cc` covers the modulo (a brute-force oracle, zero and negative moduli), the sticky
  overflow flag with unsigned saturation, and the `min`/`max` ranges.

## Verification and benchmarks

CI (`.github/workflows/ci.yml`) calls the catalog's shared
[`cmake.yml`](https://github.com/Kronuz/cpp-libraries/blob/main/.github/workflows/cmake.yml) on every push and nightly:
CTest on Linux (x86-64 GCC and Clang, arm64), macOS (arm64, x86-64) and FreeBSD, then again under
ASan+UBSan and TSan. The shared method is the catalog's
[PERFORMANCE.md](https://github.com/Kronuz/cpp-libraries/blob/main/docs/PERFORMANCE.md).

```sh
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
# ASan+UBSan as CI's linux-asan-ubsan job runs it (Linux, Clang)
CC=clang CXX=clang++ cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS" \
  "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined"
cmake --build build-asan && ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build-asan --output-on-failure
# Revert check of the 2026-11-01 fixes: the header before them must fail the test
git show 2730483:math.hh > math.hh && cmake --build build && ctest --test-dir build; git checkout math.hh
```

CTest runs `math` and `math_ubsan`, the same `test/test.cc` built with `-fsanitize=undefined
-fno-sanitize-recover=all` (GCC and Clang), so a negated `INT_MIN` aborts it in every build. The
oracles live in `test/test.cc`: a brute-force residue for every `val` in [-60, 60] and `mod` in
[1, 20], the least non-negative residue in `__int128` for the lowest signed values and 200,000
random `int64_t`, and the exact sum or difference in a wider type for every `uint8_t` pair and
random `uint16_t`, `uint32_t` and `uint64_t` pairs. There is no bench target, fuzz target or
mutation script: the library makes no performance claim.
