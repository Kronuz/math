# Architecture

A flat header of small, self-contained integer-math helpers. No state, no allocation.
Each function is a few lines; the value is in getting the edge cases right.

## The functions

- **`modulus(val, mod)`** — the mathematical modulo, always in `[0, mod)`. C++'s `%`
  keeps the sign of the dividend (`-1 % 3 == -1`); `modulus` folds negatives back into
  range (`modulus(-1, 3) == 2`). Throws `std::invalid_argument` on a non-positive
  modulus, zero included. It computes on unsigned values in the wider of the two types, so
  the lowest value of a signed type (`INT_MIN`, whose negation does not fit) and a `val`
  outside the range of a narrower modulus type give the residue.
- **`add(x, y, overflow)` / `sub(x, y, overflow)`** — wrap-around-detecting add/sub.
  The check (`x + y < x`) is a genuine wrap test **for unsigned types only** (signed
  overflow is UB), formed in the type itself (`uint8_t` and `uint16_t` are promoted to
  `int` by `+`, which would never wrap); on overflow the flag is set and the result saturates to the type's
  max / min. The flag is sticky: a call that does not overflow leaves it as it found
  it, so `add(add(a, b, o), c, o)` reports the overflow of either call, and the
  caller starts the flag at `false`.
- **`min<T>(accuracy)` / `max<T>(accuracy)`** — the representable range for a bucketed
  accuracy list, specialized for `double` and `long`. For `double` the range runs from
  `std::numeric_limits<double>::lowest()` to `max()`.

## Dependencies

**None.** Standard library only (`<cstdint>`, `<limits>`, `<vector>`, `<type_traits>`,
`<stdexcept>`). The extraction added the first three includes, which the in-tree header
had been getting transitively from Xapiand — a latent self-containment bug, now fixed.

## Invariants

- `add`/`sub` are unsigned-only. Do not use them on signed types.
- `add`/`sub` never clear the overflow flag; chained calls rely on it.
- `modulus` throws on a non-positive modulus, zero included; keep that contract.
- Verbatim from Xapiand (behavior), self-contained includes (the fix).
