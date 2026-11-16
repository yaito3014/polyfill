#ifndef YK_ZZ_POLYFILL_NEW_HPP
#define YK_ZZ_POLYFILL_NEW_HPP

#include <new>
#include <type_traits>

namespace yk {

namespace polyfill {

// Before C++17, returns p: C++14's [basic.compound] makes a pointer whose value is the address of an object point to
// that object "regardless of how the value was obtained". The need for launder came with P0137R1, which tied
// pointer values to objects.
template<class T>
constexpr T* launder(T* p) noexcept
{
  static_assert(!std::is_function<T>::value && !std::is_void<T>::value, "[ptr.launder] mandates !is_function_v<T> && !is_void_v<T>");
#if __cpp_lib_launder >= 201606L
  return std::launder(p);
#else
  return p;
#endif
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_NEW_HPP
