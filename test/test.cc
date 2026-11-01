// ctest for math: the always-non-negative modulus and the overflow-detecting add/sub.

#include "math.hh"

#include <cstdint>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <utility>
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

	// add() and sub() wrap in the type itself. For an unsigned type narrower than int, x + y is promoted
	// to int, which never wraps, so `x + y < x` was never true and uint8_t and uint16_t never reported
	// an overflow. Oracle: the exact result in a wider type (add overflows when the sum exceeds the
	// type's maximum, sub when y > x) and the saturated value.
	{
		bool all = true;
		int shown = 0;
		for (int x = 0; x < 256; ++x) {
			for (int y = 0; y < 256; ++y) {
				const auto a = static_cast<std::uint8_t>(x), b = static_cast<std::uint8_t>(y);
				bool o = false, u = false;
				const std::uint8_t s = add(a, b, o);
				const std::uint8_t d = sub(a, b, u);
				const bool want_o = x + y > 255, want_u = y > x;
				if (o != want_o || s != (want_o ? 255 : x + y) || u != want_u || d != (want_u ? 0 : x - y)) {
					if (shown++ < 10) { std::fprintf(stderr, "FAIL: uint8_t %d, %d: add %d (%d), sub %d (%d)\n", x, y, s, o, d, u); }
					all = false;
				}
			}
		}
		check(all, "add/sub on every pair of uint8_t: result and flag match the exact value");

		all = true;
		std::uint64_t seed = 0x9e3779b97f4a7c15ULL;
		auto next = [&seed] { seed ^= seed << 13; seed ^= seed >> 7; seed ^= seed << 17; return seed; };
		const std::vector<int> edges{0, 1, 2, 255, 256, 257, 32767, 32768, 32769, 65534, 65535};
		std::vector<std::pair<int, int>> pairs;
		for (int x : edges) { for (int y : edges) { pairs.emplace_back(x, y); } }
		for (int i = 0; i < 2000000; ++i) {
			const std::uint64_t r = next();
			pairs.emplace_back(static_cast<int>(r & 0xffff), static_cast<int>((r >> 20) & 0xffff));
		}
		for (auto [x, y] : pairs) {
			const auto a = static_cast<std::uint16_t>(x), b = static_cast<std::uint16_t>(y);
			bool o = false, u = false;
			const std::uint16_t s = add(a, b, o);
			const std::uint16_t d = sub(a, b, u);
			const bool want_o = x + y > 65535, want_u = y > x;
			if (o != want_o || s != (want_o ? 65535 : x + y) || u != want_u || d != (want_u ? 0 : x - y)) { all = false; }
		}
		check(all, "add/sub on the edges and 2,000,000 random pairs of uint16_t match the exact value");

		bool o = false;
		check(add(static_cast<std::uint8_t>(250), static_cast<std::uint8_t>(10), o) == 255 && o, "250 + 10 overflows a uint8_t and saturates to 255");
		bool u = false;
		check(sub(static_cast<std::uint16_t>(0), static_cast<std::uint16_t>(1), u) == 0 && u, "0 - 1 underflows a uint16_t and saturates to 0");
		bool clear = false;
		check(add(static_cast<std::uint8_t>(100), static_cast<std::uint8_t>(100), clear) == 200 && !clear, "100 + 100 fits a uint8_t");

		// The wider types keep their results (uint32_t against a uint64_t sum, uint64_t against the exact test).
		all = true;
		for (int i = 0; i < 1000000; ++i) {
			const std::uint64_t r = next();
			const auto a = static_cast<std::uint32_t>(r), b = static_cast<std::uint32_t>(r >> 32);
			bool ov = false, un = false;
			const std::uint32_t s = add(a, b, ov), d = sub(a, b, un);
			const std::uint64_t wide = static_cast<std::uint64_t>(a) + b;
			if (ov != (wide > 0xffffffffULL) || s != (ov ? 0xffffffffU : static_cast<std::uint32_t>(wide)) || un != (b > a) || d != (un ? 0U : a - b)) { all = false; }
			const std::uint64_t big = next(), small = next();
			bool ov64 = false, un64 = false;
			const std::uint64_t s64 = add(big, small, ov64), d64 = sub(big, small, un64);
			if (ov64 != (small > ~big) || s64 != (ov64 ? ~0ULL : big + small) || un64 != (small > big) || d64 != (un64 ? 0ULL : big - small)) { all = false; }
		}
		check(all, "add/sub on uint32_t and uint64_t match the exact value");
	}

	// modulus() computes in unsigned arithmetic: -val does not fit for the lowest value of a signed type
	// (undefined behavior for int, a wrong result for int8_t and int16_t, whose negation is narrowed
	// back), and static_cast<M>(val) truncated a value outside the range of a narrower M before taking
	// the remainder. Oracle: the least non-negative residue in 128 bits.
	{
		__extension__ typedef __int128 i128;
		auto ref = [](i128 v, i128 m) { i128 r = v % m; return r < 0 ? r + m : r; };
		auto agrees = [&](auto v, auto m) { return static_cast<i128>(modulus(v, m)) == ref(static_cast<i128>(v), static_cast<i128>(m)); };

		bool all = true;
		const int int_mods[] = {1, 2, 3, 7, 10, 255, 65536, 1000000007, std::numeric_limits<int>::max()};
		for (int m : int_mods) {
			all = all && agrees(std::numeric_limits<int>::min(), m) && agrees(std::numeric_limits<int>::min(), static_cast<unsigned>(m));
			all = all && agrees(std::numeric_limits<int>::min() + 1, m) && agrees(std::numeric_limits<int>::max(), m);
			all = all && agrees(std::numeric_limits<long>::min(), static_cast<long>(m)) && agrees(std::numeric_limits<long long>::min(), static_cast<unsigned long long>(m));
		}
		check(all, "modulus of INT_MIN, LONG_MIN and LLONG_MIN equals the residue");
		check(modulus(std::numeric_limits<int>::min(), 7) == 5, "INT_MIN mod 7 == 5");
		check(modulus(std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max()) == std::numeric_limits<long long>::max() - 1, "LLONG_MIN mod LLONG_MAX == LLONG_MAX - 1");

		all = true;
		for (int v = -128; v <= 127; ++v) {
			for (int m = 1; m <= 300; ++m) {
				all = all && agrees(static_cast<std::int8_t>(v), m) && agrees(static_cast<std::int8_t>(v), static_cast<long>(m));
				if (m <= 255) { all = all && agrees(static_cast<std::int8_t>(v), static_cast<std::uint8_t>(m)) && agrees(v, static_cast<std::uint8_t>(m)); }
			}
		}
		check(all, "modulus of every int8_t, int and uint8_t-modulus pair equals the residue");
		check(modulus(static_cast<std::int8_t>(-128), 7) == 5, "-128 (int8_t) mod 7 == 5");
		check(modulus(static_cast<std::int16_t>(-32768), 1000) == 232, "INT16_MIN mod 1000 == 232");

		// A value outside the range of the modulus type: 300 is 44 as a uint8_t.
		check(modulus(300, static_cast<std::uint8_t>(7)) == 6 && modulus(-300, static_cast<std::uint8_t>(7)) == 1, "300 mod uint8_t(7) == 6, -300 mod uint8_t(7) == 1");
		check(modulus(std::numeric_limits<int>::min(), static_cast<std::uint8_t>(7)) == 5, "INT_MIN mod uint8_t(7) == 5");

		all = true;
		std::uint64_t seed = 88172645463325252ULL;
		for (int i = 0; i < 200000; ++i) {
			seed ^= seed << 13; seed ^= seed >> 7; seed ^= seed << 17;
			const auto v = static_cast<std::int64_t>(seed);
			const auto m = static_cast<std::int64_t>((seed >> 11) % 100000) + 1;
			all = all && agrees(v, m) && agrees(v, static_cast<std::uint64_t>(m)) && agrees(static_cast<std::int32_t>(v), static_cast<std::int32_t>(m));
		}
		check(all, "modulus of 200,000 random int64_t values equals the residue");
	}

	if (failures == 0) { std::puts("all math tests passed"); }
	return failures == 0 ? 0 : 1;
}
