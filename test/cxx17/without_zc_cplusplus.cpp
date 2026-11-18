// Compiled by MSVC without /Zc:__cplusplus, as by a consumer that does not use the yk::polyfill target, so that only
// _MSVC_LANG reports the language version.

#include <yk/polyfill/bit.hpp>
#include <yk/polyfill/concepts.hpp>
#include <yk/polyfill/expected.hpp>
#include <yk/polyfill/extension/toptional.hpp>
#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/indirect.hpp>
#include <yk/polyfill/memory.hpp>
#include <yk/polyfill/new.hpp>
#include <yk/polyfill/optional.hpp>
#include <yk/polyfill/polymorphic.hpp>
#include <yk/polyfill/tuple.hpp>
#include <yk/polyfill/type_traits.hpp>
#include <yk/polyfill/utility.hpp>
#include <yk/polyfill/variant.hpp>

#include <type_traits>

#if _MSVC_LANG >= 202002L
#  include <compare>
#endif

static_assert(__cplusplus == 199711L, "this check needs __cplusplus to stay at its default");

namespace {

int answer() { return 42; }

static_assert(yk::polyfill::constant_wrapper<42>::value == 42, "constant_wrapper needs C++17");
static_assert(
    std::is_constructible<yk::polyfill::function_ref<int()>, yk::polyfill::constant_wrapper<&answer>>::value,
    "function_ref's constant_wrapper constructors need C++17"
);
static_assert(std::is_same<decltype(yk::polyfill::indirect(1)), yk::polyfill::indirect<int>>::value, "indirect's deduction guides need C++17");

#if _MSVC_LANG >= 202002L
static_assert(std::three_way_comparable<yk::polyfill::optional<int>>, "optional's operator<=> needs C++20");
static_assert(std::three_way_comparable<yk::polyfill::variant<int>>, "variant's operator<=> needs C++20");
static_assert(std::three_way_comparable<yk::polyfill::unique_ptr<int>>, "unique_ptr's operator<=> needs C++20");
#endif

}  // namespace
