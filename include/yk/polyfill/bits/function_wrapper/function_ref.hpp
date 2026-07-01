#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_HPP

#include "yk/polyfill/bits/function_wrapper/common.hpp"

namespace yk {

namespace polyfill {

template<class Signature>
class function_ref;

namespace detail {

template<bool Const>
struct cv_int_ref;

template<>
struct cv_int_ref</* Const = */ false> {
  using type = int&;
};

template<>
struct cv_int_ref</* Const = */ true> {
  using type = int const&;
};

template<bool Noexcept, class R, class... Args>
struct fn_ref {
  using type = R (&)(Args...);
};

#if __cpp_noexcept_function_type >= 201510L
template<class R, class... Args>
struct fn_ref</* Noexcept = */ true, R, Args...> {
  using type = R (&)(Args...) noexcept;
};
#endif

template<bool CurConst, bool CurNoexcept, bool Cv2Const, bool Noex2, class R, class... Args>
struct function_ref_spec_convertible
    : conjunction<std::is_convertible<typename fn_ref<Noex2, R, Args...>::type, typename fn_ref<CurNoexcept, R, Args...>::type>,
                  std::is_convertible<typename cv_int_ref<CurConst>::type, typename cv_int_ref<Cv2Const>::type>> {};

template<class F, bool CurConst, bool CurNoexcept, class R, class... Args>
struct is_convertible_from_function_ref_specialization : std::false_type {};

template<bool CurConst, bool CurNoexcept, class R, class... Args>
struct is_convertible_from_function_ref_specialization<function_ref<R(Args...)>, CurConst, CurNoexcept, R, Args...>
    : function_ref_spec_convertible<CurConst, CurNoexcept, /* Cv2Const = */ false, /* Noex2 = */ false, R, Args...> {};

template<bool CurConst, bool CurNoexcept, class R, class... Args>
struct is_convertible_from_function_ref_specialization<function_ref<R(Args...) const>, CurConst, CurNoexcept, R, Args...>
    : function_ref_spec_convertible<CurConst, CurNoexcept, /* Cv2Const = */ true, /* Noex2 = */ false, R, Args...> {};

#if __cpp_noexcept_function_type >= 201510L

template<bool CurConst, bool CurNoexcept, class R, class... Args>
struct is_convertible_from_function_ref_specialization<function_ref<R(Args...) noexcept>, CurConst, CurNoexcept, R, Args...>
    : function_ref_spec_convertible<CurConst, CurNoexcept, /* Cv2Const = */ false, /* Noex2 = */ true, R, Args...> {};

template<bool CurConst, bool CurNoexcept, class R, class... Args>
struct is_convertible_from_function_ref_specialization<function_ref<R(Args...) const noexcept>, CurConst, CurNoexcept, R, Args...>
    : function_ref_spec_convertible<CurConst, CurNoexcept, /* Cv2Const = */ true, /* Noex2 = */ true, R, Args...> {};

#endif

}  // namespace detail

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
