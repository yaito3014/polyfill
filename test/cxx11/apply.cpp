#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/apply.hpp>
#include <yk/polyfill/cxx11/integral_constant.hpp>

#include <string>
#include <tuple>
#include <utility>

#include <cstddef>

namespace pf = yk::polyfill;

TEST_CASE("apply")
{
  {
    auto func = [](int i, double d) -> std::string { return std::to_string(i) + "_" + std::to_string(d); };
    CHECK(pf::apply(func, std::make_pair(42, 3.14)) == "42_3.140000");
  }
  {
    auto func = [](int i, double d, std::string s) -> std::string { return std::to_string(i) + "_" + std::to_string(d) + "_" + s; };
    CHECK(pf::apply(func, std::make_tuple(42, 3.14, "foo")) == "42_3.140000_foo");
  }
}
