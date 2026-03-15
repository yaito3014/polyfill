#ifndef YK_POLYFILL_MEMORY_HPP
#define YK_POLYFILL_MEMORY_HPP

#include <yk/polyfill/config.hpp>
#include <yk/polyfill/type_traits.hpp>

#include <memory>
#include <type_traits>

#include <cstddef>

namespace yk {

namespace polyfill {

template<class T, class... Args, class = typename std::enable_if<!std::is_array<T>::value>::type>
[[nodiscard]] constexpr std::unique_ptr<T> make_unique(Args&&... args)
{
  return std::unique_ptr<T>(new T(static_cast<Args&&>(args)...));
}

template<class T, class = typename std::enable_if<is_unbounded_array<T>::value>::type>
[[nodiscard]] constexpr std::unique_ptr<T> make_unique(std::size_t size)
{
  return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]);
}

template<class T, class... Args, class = typename std::enable_if<is_bounded_array<T>::value>::type>
constexpr void make_unique(Args&&...) = delete;

template<class T, class... Args>
YK_POLYFILL_CXX20_CONSTEXPR void construct_at(T* dest, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
{
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
  std::construct_at(dest, std::forward<Args>(args)...);
#else
  new (dest) T(std::forward<Args>(args)...);
#endif
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_MEMORY_HPP
