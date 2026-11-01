# Changelog

Notable changes to `math`, newest first. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); the library has no releases, so
entries are dated.

## 2026-11-01

### Fixed

- `add` and `sub` on `uint8_t` and `uint16_t` never reported an overflow. `x + y` is formed in
  `int` for a type narrower than `int`, which does not wrap, so `x + y < x` was never true
  (`add<uint8_t>(250, 10)` returned 4 and left the flag clear, and `sub<uint8_t>(0, 1)` returned
  255). The sum and the difference are now formed in the type, so they wrap and are tested
  there; `uint32_t` and `uint64_t` are as before. Oracle: the exact result in a wider type (the
  sum exceeds the maximum, or `y > x`), compared over every pair of `uint8_t`, the edges and
  2,000,000 random pairs of `uint16_t`, and 1,000,000 random pairs of `uint32_t` and `uint64_t`.
- `modulus(INT_MIN, m)` negated `INT_MIN`, which is undefined behavior (UBSan reports it:
  `negation of -2147483648 cannot be represented in type 'int'`), and on some builds the
  residue came out wrong (`modulus(INT_MIN, 7)` was not 5). The same held for `LONG_MIN` and
  `LLONG_MIN`, and, without undefined behavior, for `int8_t` and `int16_t`, whose negation is
  narrowed back (`modulus(INT16_MIN, 1000)` was not 232). `modulus`
  takes the magnitude of a
  negative `val` as an unsigned value, in the wider of the two types. The same change fixes
  `static_cast<M>(val) % mod` for a modulus type narrower than `val`'s: a `val` outside its
  range was reduced after the conversion (`modulus(300, uint8_t{7})` was 2, not 6). Results for
  every other pair are unchanged. Oracle: the least non-negative residue in 128 bits, compared
  over `INT_MIN`, `LONG_MIN` and `LLONG_MIN` with several moduli, every `int8_t` with moduli
  1 to 300 in four modulus types, and 200,000 random `int64_t`.
- `modulus(val, 0)` divided by zero (undefined behavior; the compilers on x86-64 Linux and
  arm64 macOS folded it away and returned without throwing) although `ARCHITECTURE.md` and
  `AGENTS.md` document that it throws on a non-positive modulus. The guard was `mod < 0`;
  it is now `!(mod > 0)`, so a zero modulus throws
  `std::invalid_argument("Modulus must be positive")` like a negative one, for every
  pair of integer types (an unsigned modulus has no negative value, but zero still throws).
  Results for a positive modulus are unchanged: `test/test.cc` compares every pair of
  `val` in [-60, 60] and `mod` in [1, 20] with a brute-force residue, for `int`, `long` and
  `unsigned` moduli.
- `min<double>` returned `std::numeric_limits<double>::min()`, the smallest positive normal
  double (about 2.2e-308), as the low end of a range whose high end is `max()`. It returns
  `std::numeric_limits<double>::lowest()`, so every finite double lies in
  `[min<double>, max<double>]`. The Xapiand range queries that need the low end of a double
  range already write `std::numeric_limits<double>::lowest()`.

### Changed

- The overflow flag of `add` and `sub` is documented as sticky: a call that does not
  overflow leaves the flag as it found it. Chains such as `add(add(a, b, o), c, o)` depend
  on it (a flag cleared on each call would lose the first overflow), and the documents
  already told callers to start the flag at `false`. The behavior is unchanged;
  `test/test.cc` now pins it. The two-argument overloads `add(x, y)` and `sub(x, y)`
  start their discarded flag at `false`, where it was uninitialized.

### Tests

- `test/test.cc` checks the zero and negative moduli of every integer type pair, the
  residues against a brute-force oracle, the sticky flag (a later call that does not
  overflow, a chained call, a flag that starts set), `min<double>` against `lowest()`,
  `-max()` and a sample of doubles from `-max()` to `max()`, `add` and `sub` on every
  unsigned width, and `modulus` of the lowest values of the signed types. The old header
  fails the zero modulus, the lowest double, the narrow `add` and `sub`, and the lowest
  values and the out-of-range `val` of `modulus`.
- `math_ubsan` builds `test/test.cc` with `-fsanitize=undefined -fno-sanitize-recover=all`
  on GCC and Clang; the old `modulus` aborts it on the negation of `INT_MIN`.
