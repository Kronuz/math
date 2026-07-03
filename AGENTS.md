# AGENTS

Small integer-math helpers extracted verbatim from Xapiand: `modulus` (always
non-negative), overflow-detecting `add`/`sub`, and `min`/`max<T>(accuracy)`.

- Header-only, standard-library only. Self-contained includes (`<cstdint>`,
  `<limits>`, `<vector>`, `<type_traits>`, `<stdexcept>`) -- the extraction added the
  first three, which Xapiand had been supplying transitively.
- `add`/`sub` overflow detection (`x + y < x`) is a wrap-around test valid for
  UNSIGNED types only (signed overflow is UB). Do not use them on signed types.
- `modulus` throws `std::invalid_argument` on a non-positive modulus.
- `test/test.cc` covers the modulo sign behavior and unsigned overflow saturation.
