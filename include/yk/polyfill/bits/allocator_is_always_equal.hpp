#ifndef YK_ZZ_POLYFILL_BITS_ALLOCATOR_IS_ALWAYS_EQUAL_HPP
#define YK_ZZ_POLYFILL_BITS_ALLOCATOR_IS_ALWAYS_EQUAL_HPP

// Fallback for is_always_equal (added to allocator_traits in C++17).

#include <memory>
#include <type_traits>

namespace yk {

namespace polyfill {

namespace detail {

#if __cpp_lib_allocator_traits_is_always_equal >= 201411L
template<class A>
struct allocator_is_always_equal : std::allocator_traits<A>::is_always_equal {};
#else
template<class A>
struct allocator_is_always_equal : std::is_empty<A> {};
#endif

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_ALLOCATOR_IS_ALWAYS_EQUAL_HPP
