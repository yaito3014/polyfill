#ifndef YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP
#define YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP

#include <yk/polyfill/type_traits.hpp>

#include <exception>

namespace yk {

namespace polyfill {

struct nullopt_t {
  struct construct_tag {
    explicit construct_tag() = default;
  };
  explicit constexpr nullopt_t(construct_tag) noexcept {}
};

struct nullopt_holder {
  static constexpr nullopt_t value{nullopt_t::construct_tag{}};
};

#if __cpp_inline_variables >= 201606L

inline constexpr nullopt_t nullopt{nullopt_t::construct_tag{}};

#endif

class bad_optional_access : public std::exception {
public:
  char const* what() const noexcept override { return "accessing empty optional"; }
};

namespace optional_detail {

template<class T, class U>
constexpr bool converts_from_any_cvref = disjunction<
    std::is_constructible<T, U&>, std::is_convertible<U&, T>, std::is_constructible<T, U>, std::is_convertible<U, T>, std::is_constructible<T, U const&>,
    std::is_convertible<U const&, T>, std::is_constructible<T, U const>, std::is_convertible<U const, T>>::value;

}  // namespace optional_detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP
