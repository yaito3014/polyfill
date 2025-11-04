#ifndef YK_POLYFILL_BITS_OPTIONAL_DETAIL_HPP
#define YK_POLYFILL_BITS_OPTIONAL_DETAIL_HPP

#include <yk/polyfill/bits/core_traits.hpp>

#include <exception>
#include <type_traits>

namespace yk {

namespace polyfill {

struct nullopt_t {
  explicit nullopt_t() noexcept = default;
};

struct nullopt_holder {
  static constexpr nullopt_t value{};
};

#if __cpp_inline_variables >= 201606L

inline constexpr nullopt_t nullopt{};

#endif

class bad_optional_access : public std::exception {
  char const* what() const noexcept { return "accessing empty optional"; }
};

namespace optional_detail {

template<class T, class W>
struct converts_from_any_cvref {
  static constexpr bool value = disjunction<
      std::is_constructible<T, W&>, std::is_convertible<T, W&>, std::is_constructible<T, W const&>, std::is_convertible<T, W const&>,
      std::is_constructible<T, W&&>, std::is_convertible<T, W&&>, std::is_constructible<T, W const&&>, std::is_convertible<T, W const&&>>::value;
};

}  // namespace optional_detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_BITS_OPTIONAL_DETAIL_HPP
