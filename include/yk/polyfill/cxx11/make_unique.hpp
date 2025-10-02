#ifndef YK_POLYFILL_CXX11_MAKE_UNIQUE_HPP
#define YK_POLYFILL_CXX11_MAKE_UNIQUE_HPP

#include <yk/polyfill/cxx11/is_bounded_array.hpp>
#include <yk/polyfill/cxx11/is_unbounded_array.hpp>

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

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_MAKE_UNIQUE_HPP
