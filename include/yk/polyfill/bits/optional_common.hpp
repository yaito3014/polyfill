#ifndef YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP
#define YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP

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

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_BITS_OPTIONAL_COMMON_HPP
