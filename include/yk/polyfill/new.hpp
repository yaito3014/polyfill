#ifndef YK_ZZ_POLYFILL_NEW_HPP
#define YK_ZZ_POLYFILL_NEW_HPP

#include <new>
#include <type_traits>

// Before C++17, returning the pointer does not give launder's guarantee. For a pointer to an object that another
// object has replaced, C++14's [basic.life] lets it refer to the new object only if the types are the same (among
// other conditions), which conflicts with [basic.compound] making a pointer point to whatever object is at its
// address; compilers follow the former (GCC devirtualizes calls through such a pointer at -O2). P0137R1 resolved
// this together with introducing std::launder. GCC and Clang provide __builtin_launder, on which their std::launder
// is built, also in earlier modes, so it is used where available. MSVC provides it only from C++17, so there, as
// with any other compiler without it, returning the pointer is the last resort.
#if __cpp_lib_launder >= 201606L
#elif defined(__has_builtin)
#if __has_builtin(__builtin_launder)
#define YK_POLYFILL_DETAIL_BUILTIN_LAUNDER(p) __builtin_launder(p)
#endif
#elif defined(__GNUC__) && __GNUC__ >= 7
// GCC 7 to 9 have the builtin but not __has_builtin.
#define YK_POLYFILL_DETAIL_BUILTIN_LAUNDER(p) __builtin_launder(p)
#endif

namespace yk {

namespace polyfill {

template<class T>
constexpr T* launder(T* p) noexcept
{
  static_assert(!std::is_function<T>::value && !std::is_void<T>::value, "[ptr.launder] mandates !is_function_v<T> && !is_void_v<T>");
#if __cpp_lib_launder >= 201606L
  return std::launder(p);
#elif defined(YK_POLYFILL_DETAIL_BUILTIN_LAUNDER)
  return YK_POLYFILL_DETAIL_BUILTIN_LAUNDER(p);
#else
  return p;
#endif
}

}  // namespace polyfill

}  // namespace yk

#undef YK_POLYFILL_DETAIL_BUILTIN_LAUNDER

#endif  // YK_ZZ_POLYFILL_NEW_HPP
