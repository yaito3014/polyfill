#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/tuple.hpp>

#include <string>
#include <tuple>
#include <utility>
#include <version>

#include <cstddef>

namespace pf = yk::polyfill;

TEST_CASE("apply")
{
  {
    auto func = [](int i, double d) -> std::string { return std::to_string(i) + "_" + std::to_string(d); };
#if __cpp_lib_to_string >= 202306L
    CHECK(pf::apply(func, std::make_pair(42, 3.14)) == "42_3.14");
#else
    CHECK(pf::apply(func, std::make_pair(42, 3.14)) == "42_3.140000");
#endif
  }
  {
    auto func = [](int i, double d, std::string s) -> std::string { return std::to_string(i) + "_" + std::to_string(d) + "_" + s; };
#if __cpp_lib_to_string >= 202306L
    CHECK(pf::apply(func, std::make_tuple(42, 3.14, "foo")) == "42_3.14_foo");
#else
    CHECK(pf::apply(func, std::make_tuple(42, 3.14, "foo")) == "42_3.140000_foo");
#endif
  }
}
