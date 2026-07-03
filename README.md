# math

Small integer-math helpers extracted from
[Xapiand](https://github.com/Kronuz/Xapiand):

- `modulus(val, mod)` -- a **mathematically correct** modulo whose result is always
  in `[0, mod)`, unlike C++`s `%` (which keeps the sign of the dividend, so
  `-1 % 3 == -1`). `modulus(-1, 3) == 2`. Throws on a non-positive modulus.
- `add(x, y, overflow)` / `sub(x, y, overflow)` -- wrap-around-detecting add/sub for
  **unsigned** types; on overflow they set the flag and saturate to the type max/min.
- `min<T>(accuracy)` / `max<T>(accuracy)` -- the representable range for a bucketed
  accuracy list (specialized for `double` / `long`).

```cpp
#include "math.hh"
auto m = modulus(-7, 3);          // 2
bool of = false;
auto s = add(0xFFFFFFFFu, 1u, of); // of == true, s saturates to UINT32_MAX
```

Header-only, standard-library only.

## Build / test
```sh
cmake -B build && cmake --build build && ctest --test-dir build
```

## Use (FetchContent)
```cmake
FetchContent_Declare(math GIT_REPOSITORY https://github.com/Kronuz/math.git GIT_TAG main)
FetchContent_MakeAvailable(math)
target_link_libraries(your_app PRIVATE math::math)
```
