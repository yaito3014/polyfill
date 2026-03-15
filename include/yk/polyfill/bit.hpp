#ifndef YK_POLYFILL_BIT_HPP
#define YK_POLYFILL_BIT_HPP

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

template<
    class To, class From,
    typename std::enable_if<
        conjunction<std::is_trivially_copyable<From>, std::is_trivially_copyable<To>, bool_constant<sizeof(From) == sizeof(To)>>::value, std::nullptr_t>::type =
        nullptr>
YK_POLYFILL_CXX20_CONSTEXPR To bit_cast(From const& from) noexcept
{
#if __cpp_lib_bit_cast >= 201806L
  return std::bit_cast<To>(from);
#else
  alignas(To) unsigned char storage[sizeof(To)];
  std::memcpy(&storage, std::addressof(from), sizeof(From));
  return *reinterpret_cast<To*>(&storage);
#endif
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_BIT_HPP
