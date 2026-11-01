// ctest for math: the always-non-negative modulus and the overflow-detecting add/sub.

#include "math.hh"

#include <cstdint>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <vector>

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

	// The overflow flag is sticky: add() and sub() set it and never clear it, so one flag
	// collects the outcome of a chain of operations.
	overflow = false;
	unsigned a1 = add(std::numeric_limits<unsigned>::max(), 1u, overflow);
	unsigned a2 = add(1u, 1u, overflow);
	check(overflow && a1 == std::numeric_limits<unsigned>::max() && a2 == 2u, "add: a later add that does not overflow leaves the flag set");
	overflow = false;
	unsigned s1 = sub(0u, 1u, overflow);
	unsigned s2 = sub(5u, 1u, overflow);
	check(overflow && s1 == 0u && s2 == 4u, "sub: a later sub that does not overflow leaves the flag set");
	overflow = false;
	unsigned chain = add(add(std::numeric_limits<unsigned>::max(), 1u, overflow), 0u, overflow);
	check(overflow && chain == std::numeric_limits<unsigned>::max(), "a chained add keeps the overflow of the inner one");
	overflow = true;
	check(add(1u, 2u, overflow) == 3u && sub(3u, 2u, overflow) == 1u && overflow, "a flag that starts set stays set");
	overflow = false;
	check(add(1u, 2u, overflow) == 3u && sub(3u, 2u, overflow) == 1u && !overflow, "a flag that starts clear stays clear without overflow");

	// A zero modulus throws std::invalid_argument, as a negative one does, for every integer type
	// pair (the unsigned ones have no negative modulus, but the zero one still divides by zero).
	auto throws_invalid = [](auto&& call) {
		try {
			call();
		} catch (const std::invalid_argument&) {
			return true;
		} catch (...) {
			return false;
		}
		return false;
	};
	check(throws_invalid([] { return modulus(5, 0); }), "modulus(5, 0) throws");
	check(throws_invalid([] { return modulus(-5, 0); }), "modulus(-5, 0) throws");
	check(throws_invalid([] { return modulus(0, 0); }), "modulus(0, 0) throws");
	check(throws_invalid([] { return modulus(5, 0L); }), "modulus(5, 0L) throws");
	check(throws_invalid([] { return modulus(5L, 0LL); }), "modulus(5L, 0LL) throws");
	check(throws_invalid([] { return modulus(5u, 0u); }), "modulus(5u, 0u) throws");
	check(throws_invalid([] { return modulus(-5, 0u); }), "modulus(-5, 0u) throws");
	check(throws_invalid([] { return modulus(5, static_cast<uint64_t>(0)); }), "modulus(5, uint64_t(0)) throws");
	check(throws_invalid([] { return modulus(5, -1); }), "modulus(5, -1) throws");
	check(throws_invalid([] { return modulus(-5, -3L); }), "modulus(-5, -3L) throws");
	check(!throws_invalid([] { return modulus(5, 1); }), "modulus(5, 1) does not throw");

	// Exhaustive on a small domain against a brute-force oracle: the least non-negative
	// residue, found by stepping val into [0, mod) one modulus at a time.
	for (int mod = 1; mod <= 20; ++mod) {
		for (int val = -60; val <= 60; ++val) {
			int expected = val;
			while (expected < 0) { expected += mod; }
			while (expected >= mod) { expected -= mod; }
			if (modulus(val, mod) != expected || modulus(val, static_cast<long>(mod)) != expected
			    || modulus(val, static_cast<unsigned>(mod)) != static_cast<unsigned>(expected)) {
				std::fprintf(stderr, "FAIL: modulus(%d, %d) = %d, expected %d\n", val, mod, modulus(val, mod), expected);
				++failures;
			}
		}
	}

	// min<double> is the lowest finite double, the other end of the range max<double> closes
	// (std::numeric_limits<double>::min() is the smallest positive normal value, not a lower bound).
	const std::vector<uint64_t> accuracy{1, 10, 100};
	check(min<double>(accuracy) == std::numeric_limits<double>::lowest(), "min<double> is numeric_limits<double>::lowest()");
	check(min<double>(accuracy) == -std::numeric_limits<double>::max(), "min<double> is -max<double>");
	check(min<double>({}) == std::numeric_limits<double>::lowest(), "min<double> of an empty accuracy list");
	check(max<double>(accuracy) == std::numeric_limits<double>::max(), "max<double> is numeric_limits<double>::max()");
	for (double x : {-std::numeric_limits<double>::max(), -1e300, -1.0, -std::numeric_limits<double>::min(), 0.0,
	                 std::numeric_limits<double>::denorm_min(), std::numeric_limits<double>::min(), 1.0, 1e300,
	                 std::numeric_limits<double>::max()}) {
		check(min<double>(accuracy) <= x && x <= max<double>(accuracy), "every finite double lies in [min<double>, max<double>]");
	}

	// The integer ranges: twice the last accuracy, negated for min, clamped to the type.
	check(min<long>(accuracy) == -200 && max<long>(accuracy) == 200, "min/max<long> are -/+ twice the last accuracy");
	check(min<long>({}) == std::numeric_limits<long>::min() && max<long>({}) == std::numeric_limits<long>::max(), "min/max<long> of an empty list");
	check(min<unsigned long>(accuracy) == 0 && max<unsigned long>(accuracy) == 200, "min/max<unsigned long>");

	if (failures == 0) { std::puts("all math tests passed"); }
	return failures == 0 ? 0 : 1;
}
