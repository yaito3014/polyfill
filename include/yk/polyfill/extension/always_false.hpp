#ifndef YK_POLYFILL_EXTENSION_ALWAYS_FALSE_HPP
#define YK_POLYFILL_EXTENSION_ALWAYS_FALSE_HPP

#include <yk/polyfill/bits/core_traits.hpp>

namespace yk {

namespace polyfill {

namespace extension {

template<class... Ts>
struct always_false : false_type {};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_EXTENSION_ALWAYS_FALSE_HPP
