#ifndef YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP
#define YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP

#include <yk/polyfill/type_traits.hpp>

#include <exception>
#include <type_traits>

namespace yk {

namespace polyfill {

struct nullopt_t {
  struct construct_tag {
    explicit construct_tag() = default;
  };
  explicit constexpr nullopt_t(construct_tag) noexcept {}
};

YK_POLYFILL_INLINE constexpr nullopt_t nullopt{nullopt_t::construct_tag{}};

class bad_optional_access : public std::exception {
public:
  char const* what() const noexcept override { return "accessing empty optional"; }
};

namespace optional_detail {

template<class T, class W>
struct converts_from_any_cvref {
  static constexpr bool value = disjunction<
      std::is_constructible<T, W&>, std::is_convertible<W&, T>, std::is_constructible<T, W const&>, std::is_convertible<W const&, T>,
      std::is_constructible<T, W&&>, std::is_convertible<W&&, T>, std::is_constructible<T, W const&&>, std::is_convertible<W const&&, T>>::value;
};

}  // namespace optional_detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP
