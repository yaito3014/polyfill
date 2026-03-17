#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_HPP

#include "yk/polyfill/bits/function_wrapper/common.hpp"

namespace yk {

namespace polyfill {

template<class Signature>
class function_ref;

}  // namespace polyfill

}  // namespace yk

#define YK_POLYFILL_INCLUDE_FUNCTION_REF

// #define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
// #define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/function_ref.ipp"
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
// #define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/function_ref.ipp"
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#if __cpp_noexcept_function_type >= 201510L

// #define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/function_ref.ipp"
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/function_ref.ipp"
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#endif

#undef YK_POLYFILL_INCLUDE_FUNCTION_REF

namespace yk {

namespace polyfill {

#if __cpp_deduction_guides >= 201703L

template<class F, typename std::enable_if<std::is_function<F>::value, std::nullptr_t>::type = nullptr>
function_ref(F*) -> function_ref<F>;

#endif

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_HPP
