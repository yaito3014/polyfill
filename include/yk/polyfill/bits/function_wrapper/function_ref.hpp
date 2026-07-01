#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_HPP

#include "yk/polyfill/bits/function_wrapper/common.hpp"

#include <yk/polyfill/extension/invocable_traits.hpp>
#include <yk/polyfill/type_traits.hpp>  // constant_wrapper

namespace yk {

namespace polyfill {

template<class Signature>
class function_ref;

namespace detail {

template<class T>
struct is_specialization_of_constant_wrapper : std::false_type {};

#if __cplusplus >= 202002L
template<xo::cw_fixed_value X, class C>
struct is_specialization_of_constant_wrapper<constant_wrapper<X, C>> : std::true_type {};

template<class Sig>
struct drop_first_param;
template<class R, class First, class... Rest>
struct drop_first_param<R(First, Rest...)> {
  using type = R(Rest...);
};
template<class R, class First, class... Rest>
struct drop_first_param<R(First, Rest...) noexcept> {
  using type = R(Rest...) noexcept;
};

// The function_ref signature deduced by the two-argument constant_wrapper guide
// [func.wrap.ref.deduct].
template<class F, class T, class = void>
struct cw_deduced_signature {};

template<class F, class T>
struct cw_deduced_signature<F, T, typename std::enable_if<std::is_member_function_pointer<F>::value>::type> {
  using type = typename extension::invocable_traits<F>::function_type;
};

// A free function's leading parameter is the bound object, so drop it.
template<class F, class T>
struct cw_deduced_signature<F, T, typename std::enable_if<std::is_pointer<F>::value && std::is_function<typename std::remove_pointer<F>::type>::value>::type> {
  using type = typename drop_first_param<typename extension::invocable_traits<F>::function_type>::type;
};

template<class F, class T>
struct cw_deduced_signature<F, T, typename std::enable_if<std::is_member_object_pointer<F>::value>::type> {
  using result = typename polyfill::invoke_result<F, T&>::type;
  using type = result() noexcept;
};
#endif

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

#if __cplusplus >= 202002L
template<auto c, class F0, typename std::enable_if<std::is_function<typename std::remove_pointer<F0>::type>::value, std::nullptr_t>::type = nullptr>
function_ref(constant_wrapper<c, F0>) -> function_ref<typename std::remove_pointer<F0>::type>;

template<auto c, class F, class T, class Sig = typename detail::cw_deduced_signature<F, T>::type>
function_ref(constant_wrapper<c, F>, T&&) -> function_ref<Sig>;
#endif

#endif

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_HPP
