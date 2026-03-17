#ifndef YK_POLYFILL_INCLUDE_FUNCTION_REF
#warning "Do not include this file directly."
#else

#ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_CONST const
#else
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_CONST
#endif

#ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_NOEXCEPT noexcept
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_IS_NOEXCEPT true
#else
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_NOEXCEPT
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_IS_NOEXCEPT false
#endif

namespace yk {

namespace polyfill {

template<class R, class... Args>
class function_ref<R(Args...) YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_CONST YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_NOEXCEPT> {
private:
  template<class... Ts>
  using is_invocable_using =
#ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
      is_nothrow_invocable_r<R, Ts..., Args...>
#else
      is_invocable_r<R, Ts..., Args...>
#endif
      ;

public:
  template<
      class F, typename std::enable_if<std::is_function<F>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<is_invocable_using<F>::value, std::nullptr_t>::type = nullptr>
  function_ref(F* f) noexcept
      : entity_(detail::bound_entity::func_tag{}, f),
        thunk_ptr_(&detail::invoker<YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_IS_NOEXCEPT, R, Args...>::template invoke_func<F>)
  {
  }

  // NOTE: Per the standard, T = remove_reference_t<F> preserves const from the source object.
  // This means a non-const signature R(Args...) accepts const objects and invokes their const operator().
  // The non-const qualifier is "const-transparent" (don't add const), not "non-const-only" (reject const).
  // Expressing "non-const-only" would require a hypothetical R(Args...) mutable function type.
  template<
      class F, typename std::enable_if<!std::is_same<typename remove_cvref<F>::type, function_ref>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!std::is_member_pointer<typename remove_cvref<F>::type>::value, std::nullptr_t>::type = nullptr,
      class T = typename std::remove_reference<F>::type,
      typename std::enable_if<is_invocable_using<T YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_CONST&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR function_ref(F&& f) noexcept
      : entity_(detail::bound_entity::obj_tag{}, std::forward<F>(f)),
        thunk_ptr_(&detail::invoker<YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_IS_NOEXCEPT, R, Args...>::template invoke_obj<
                   YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_CONST T>)
  {
  }

  constexpr function_ref(function_ref const&) noexcept = default;
  YK_POLYFILL_CXX14_CONSTEXPR function_ref& operator=(function_ref const&) noexcept = default;

  template<
      class T, typename std::enable_if<!std::is_same<T, function_ref>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!std::is_pointer<T>::value, std::nullptr_t>::type = nullptr>
  function_ref& operator=(T) = delete;

  R operator()(Args... args) const YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_NOEXCEPT { return thunk_ptr_(entity_, std::forward<Args>(args)...); }

private:
  detail::bound_entity entity_;
  R (*thunk_ptr_)(detail::bound_entity, Args&&...) YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_NOEXCEPT;
};

}  // namespace polyfill

}  // namespace yk

#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_NOEXCEPT
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_FUNCTION_REF_IS_NOEXCEPT

#endif
