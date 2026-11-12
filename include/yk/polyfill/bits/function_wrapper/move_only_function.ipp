#ifndef YK_POLYFILL_INCLUDE_MOVE_ONLY_FUNCTION
#warning "Do not include this file directly."
#else

// Expanded once per specialization of [func.wrap.move.general], selected by the APPLY_* macros,
// which this file undefines at the end.

#ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV const
#else
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV
#endif

// inv-quals is cv& when ref is empty, otherwise cv ref
#if defined(YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF)
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_REF &
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_INV_QUALS YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV&
#elif defined(YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF)
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_REF &&
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_INV_QUALS YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV&&
#else
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_REF
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_INV_QUALS YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV&
#endif

#ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT noexcept
#else
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT
#endif

namespace yk {

namespace polyfill {

template<class R, class... Args>
class move_only_function<
    R(Args...) YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_REF YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT> {
private:
  template<class T>
  using is_invocable_using =
#ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
      is_nothrow_invocable_r<R, T, Args...>
#else
      is_invocable_r<R, T, Args...>
#endif
      ;

  template<class VT>
  using is_callable_from = conjunction<
      is_invocable_using<VT YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_REF>,
      is_invocable_using<VT YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_INV_QUALS>>;

  template<class VT>
  static R invoke_target(detail::mof_storage const& storage, Args&&... args) YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT
  {
    auto&& f = detail::mof_holder<VT>::get(storage);
    return polyfill::invoke_r<R>(static_cast<VT YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_INV_QUALS>(f), std::forward<Args>(args)...);
  }

public:
  using result_type = R;

  move_only_function() noexcept = default;

  move_only_function(std::nullptr_t) noexcept {}

  move_only_function(move_only_function&& other) noexcept : storage_(other.storage_), invoke_(other.invoke_), destroy_(other.destroy_)
  {
    other.invoke_ = nullptr;
    other.destroy_ = nullptr;
  }

  template<
      class F, class VT = typename std::decay<F>::type,
      typename std::enable_if<!std::is_same<typename remove_cvref<F>::type, move_only_function>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!extension::is_specialization_of<typename remove_cvref<F>::type, in_place_type_t>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<is_callable_from<VT>::value, std::nullptr_t>::type = nullptr>
  move_only_function(F&& f)
  {
    static_assert(std::is_constructible<VT, F>::value, "[func.wrap.move.ctor] mandates is_constructible_v<VT, F>");
    if (detail::mof_is_null<typename remove_cvref<F>::type>(f)) return;
    detail::mof_holder<VT>::create(storage_, std::forward<F>(f));
    invoke_ = &invoke_target<VT>;
    destroy_ = &detail::mof_holder<VT>::destroy;
  }

  ~move_only_function()
  {
    if (invoke_) destroy_(storage_);
  }

  explicit operator bool() const noexcept { return invoke_ != nullptr; }

  R operator()(Args... args) YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_REF YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT
  {
    return invoke_(storage_, std::forward<Args>(args)...);
  }

private:
  detail::mof_storage storage_ = {nullptr};
  R (*invoke_)(detail::mof_storage const&, Args&&...) YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT = nullptr;
  void (*destroy_)(detail::mof_storage&) noexcept = nullptr;
};

}  // namespace polyfill

}  // namespace yk

#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_REF
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_INV_QUALS
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT

#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#endif
