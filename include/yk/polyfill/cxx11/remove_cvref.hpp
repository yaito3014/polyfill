#ifndef YK_POLYFILL_CXX11_REMOVE_CVREF_HPP
#define YK_POLYFILL_CXX11_REMOVE_CVREF_HPP

#include <type_traits>

namespace yk {

namespace polyfill {

template<class T>
struct remove_cvref {
  using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_REMOVE_CVREF_HPP
