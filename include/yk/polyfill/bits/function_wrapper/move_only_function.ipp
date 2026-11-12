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

  template<class VT, class... CArgs>
  void construct_target(CArgs&&... args)
  {
    detail::mof_holder<VT>::create(storage_, std::forward<CArgs>(args)...);
    invoke_ = &invoke_target<VT>;
    manage_ = &detail::mof_holder<VT>::manage;
  }

  // Moves the target object of other, if any, into *this, which has none, leaving other with none.
  void take(move_only_function& other) noexcept
  {
    if (other.manage_) other.manage_(other.storage_, &storage_);
    invoke_ = other.invoke_;
    manage_ = other.manage_;
    other.invoke_ = nullptr;
    other.manage_ = nullptr;
  }

public:
  using result_type = R;

  move_only_function() noexcept = default;

  move_only_function(std::nullptr_t) noexcept {}

  move_only_function(move_only_function&& other) noexcept { take(other); }

  template<
      class F, class VT = typename std::decay<F>::type,
      typename std::enable_if<!std::is_same<typename remove_cvref<F>::type, move_only_function>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!extension::is_specialization_of<typename remove_cvref<F>::type, in_place_type_t>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<is_callable_from<VT>::value, std::nullptr_t>::type = nullptr>
  move_only_function(F&& f)
  {
    static_assert(std::is_constructible<VT, F>::value, "[func.wrap.move.ctor] mandates is_constructible_v<VT, F>");
    if (detail::mof_is_null<typename remove_cvref<F>::type>(f)) return;
    construct_target<VT>(std::forward<F>(f));
  }

  template<
      class T, class... CArgs, class VT = typename std::decay<T>::type,
      typename std::enable_if<conjunction<std::is_constructible<VT, CArgs...>, is_callable_from<VT>>::value, std::nullptr_t>::type = nullptr>
  explicit move_only_function(in_place_type_t<T>, CArgs&&... args)
  {
    static_assert(std::is_same<VT, T>::value, "[func.wrap.move.ctor] mandates that VT is the same type as T");
    construct_target<VT>(std::forward<CArgs>(args)...);
  }

  template<
      class T, class U, class... CArgs, class VT = typename std::decay<T>::type,
      typename std::enable_if<conjunction<std::is_constructible<VT, std::initializer_list<U>&, CArgs...>, is_callable_from<VT>>::value, std::nullptr_t>::type =
          nullptr>
  explicit move_only_function(in_place_type_t<T>, std::initializer_list<U> ilist, CArgs&&... args)
  {
    static_assert(std::is_same<VT, T>::value, "[func.wrap.move.ctor] mandates that VT is the same type as T");
    construct_target<VT>(ilist, std::forward<CArgs>(args)...);
  }

  // noexcept is a strengthening allowed by [res.on.exception.handling], as moving a target never throws here.
  move_only_function& operator=(move_only_function&& other) noexcept
  {
    move_only_function(std::move(other)).swap(*this);
    return *this;
  }

  move_only_function& operator=(std::nullptr_t) noexcept
  {
    if (manage_) manage_(storage_, nullptr);
    invoke_ = nullptr;
    manage_ = nullptr;
    return *this;
  }

  // Constrained as in libstdc++ and MSVC STL, although [func.wrap.move.ctor] does not, so that
  // move_only_function is not reported as assignable from what it cannot be constructed from (e.g. copies).
  template<class F, typename std::enable_if<std::is_constructible<move_only_function, F>::value, std::nullptr_t>::type = nullptr>
  move_only_function& operator=(F&& f)
  {
    move_only_function(std::forward<F>(f)).swap(*this);
    return *this;
  }

  ~move_only_function()
  {
    if (manage_) manage_(storage_, nullptr);
  }

  explicit operator bool() const noexcept { return invoke_ != nullptr; }

  R operator()(Args... args) YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_CV YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_REF YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT
  {
    return invoke_(storage_, std::forward<Args>(args)...);
  }

  void swap(move_only_function& other) noexcept
  {
    move_only_function tmp(std::move(other));
    other.take(*this);
    take(tmp);
  }

  friend void swap(move_only_function& f1, move_only_function& f2) noexcept { f1.swap(f2); }

  friend bool operator==(move_only_function const& f, std::nullptr_t) noexcept { return !f; }

#if !(__cpp_impl_three_way_comparison >= 201907L)
  // Since C++20, these are rewritten from the operator== above.
  friend bool operator==(std::nullptr_t, move_only_function const& f) noexcept { return !f; }
  friend bool operator!=(move_only_function const& f, std::nullptr_t) noexcept { return static_cast<bool>(f); }
  friend bool operator!=(std::nullptr_t, move_only_function const& f) noexcept { return static_cast<bool>(f); }
#endif

private:
  detail::mof_storage storage_ = {nullptr};
  R (*invoke_)(detail::mof_storage const&, Args&&...) YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOF_NOEXCEPT = nullptr;
  void (*manage_)(detail::mof_storage&, detail::mof_storage*) noexcept = nullptr;
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
