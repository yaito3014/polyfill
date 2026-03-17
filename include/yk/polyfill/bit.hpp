#ifndef YK_ZZ_POLYFILL_BIT_HPP
#define YK_ZZ_POLYFILL_BIT_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/type_traits.hpp>

#include <memory>
#include <type_traits>

#include <cstring>

#if __cpp_lib_bit_cast >= 201806L
#include <bit>
#endif

namespace yk {

namespace polyfill {

// `dst` second parameter is a scratch space for memcpy destination in the pre-C++20 fallback.
// Supply an arbitrary `To` value to bypass the default construction requirement for non-default-constructible types.
// Ignored when std::bit_cast is available.
template<
    class To, class From,
    typename std::enable_if<
        conjunction<std::is_trivially_copyable<From>, std::is_trivially_copyable<To>, bool_constant<sizeof(From) == sizeof(To)>>::value, std::nullptr_t>::type =
        nullptr>
YK_POLYFILL_CXX20_CONSTEXPR To bit_cast(From const& from, To dst = To{}) noexcept
{
#if __cpp_lib_bit_cast >= 201806L
  (void)dst;
  return std::bit_cast<To>(from);
#else
  std::memcpy(std::addressof(dst), std::addressof(from), sizeof(To));
  return dst;
#endif
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BIT_HPP
