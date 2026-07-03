// ctest for math: the always-non-negative modulus and the overflow-detecting add/sub.

#include "math.hh"

#include <cstdio>
#include <limits>

int main() {
	int failures = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { std::fprintf(stderr, "FAIL: %s\n", what); ++failures; }
	};

	// modulus() is the mathematical modulo: the result is always in [0, mod).
	check(modulus(7, 3) == 1, "7 mod 3");
	check(modulus(-1, 3) == 2, "-1 mod 3 == 2 (not -1)");
	check(modulus(-3, 3) == 0, "-3 mod 3 == 0");
	check(modulus(-7, 3) == 2, "-7 mod 3 == 2");
	check(modulus(0, 5) == 0, "0 mod 5");

	// add() / sub() detect wrap-around for UNSIGNED types (signed overflow is UB);
	// on overflow they set the flag and saturate to the type's max/min.
	bool overflow = false;
	unsigned r = add(2u, 3u, overflow);
	check(r == 5u && !overflow, "2 + 3 = 5, no overflow");

	overflow = false;
	unsigned hi = add(std::numeric_limits<unsigned>::max(), 1u, overflow);
	check(overflow && hi == std::numeric_limits<unsigned>::max(), "UINT_MAX + 1 overflows, saturates to max");

	overflow = false;
	unsigned s = sub(10u, 4u, overflow);
	check(s == 6u && !overflow, "10 - 4 = 6, no overflow");

	overflow = false;
	unsigned lo = sub(0u, 1u, overflow);
	check(overflow && lo == std::numeric_limits<unsigned>::min(), "0 - 1 underflows, saturates to min");

	if (failures == 0) { std::puts("all math tests passed"); }
	return failures == 0 ? 0 : 1;
}
