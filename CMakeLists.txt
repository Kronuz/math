# math: small integer math helpers (a mathematically-correct modulus, ...)
#
#   cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build && ctest --test-dir build
#
# Header-only, dependency-free. Standard-library only.

cmake_minimum_required(VERSION 3.16)
project(math LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

add_library(math INTERFACE)
target_include_directories(math INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(math INTERFACE cxx_std_20)
add_library(math::math ALIAS math)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	enable_testing()
	add_executable(math_test test/test.cc)
	target_link_libraries(math_test PRIVATE math)
	add_test(NAME math COMMAND math_test)

	# The same test under UBSan (aborting on the first report): the lowest value of a signed type
	# must not be negated, and narrow types must not overflow in a shift or a conversion.
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		add_executable(math_test_ubsan test/test.cc)
		target_link_libraries(math_test_ubsan PRIVATE math)
		target_compile_options(math_test_ubsan PRIVATE -g -O1 -fsanitize=undefined -fno-sanitize-recover=all)
		target_link_options(math_test_ubsan PRIVATE -fsanitize=undefined)
		add_test(NAME math_ubsan COMMAND math_test_ubsan)
	endif()
endif()
