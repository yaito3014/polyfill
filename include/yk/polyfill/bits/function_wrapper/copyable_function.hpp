#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_COPYABLE_FUNCTION_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_COPYABLE_FUNCTION_HPP

#include "yk/polyfill/bits/function_wrapper/owning_common.hpp"

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/bits/invoke.hpp>
#include <yk/polyfill/extension/specialization_of.hpp>
#include <yk/polyfill/utility.hpp>  // in_place_type_t

#include <initializer_list>
#include <type_traits>
#include <utility>

#include <cstddef>

#define YK_POLYFILL_INCLUDE_COPYABLE_FUNCTION

#include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#if __cpp_noexcept_function_type >= 201510L

#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#  include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#  include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#  include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#  include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#  include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#  define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#  include "yk/polyfill/bits/function_wrapper/copyable_function.ipp"

#endif

#undef YK_POLYFILL_INCLUDE_COPYABLE_FUNCTION

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_COPYABLE_FUNCTION_HPP
