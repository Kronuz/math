# Changelog

Notable changes to `math`, newest first. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); the library has no releases, so
entries are dated.

## 2026-11-01

### Fixed

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
  overflow, a chained call, a flag that starts set), and `min<double>` against `lowest()`,
  `-max()` and a sample of doubles from `-max()` to `max()`. The old header fails it on
  the zero modulus, the lowest double, and nothing else.
