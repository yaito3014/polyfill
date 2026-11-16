#ifndef YK_POLYFILL_INCLUDE_COPYABLE_FUNCTION
#  warning "Do not include this file directly."
#else

// Expanded once per specialization of [func.wrap.copy.general], selected by the APPLY_* macros,
// which this file undefines at the end.

#  ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV const
#  else
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV
#  endif

// inv-quals is cv& when ref is empty, otherwise cv ref
#  if defined(YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF)
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_REF &
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_INV_QUALS YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV&
#  elif defined(YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF)
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_REF &&
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_INV_QUALS YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV&&
#  else
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_REF
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_INV_QUALS YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV&
#  endif

#  ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_NOEXCEPT noexcept
#  else
#    define YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_NOEXCEPT
#  endif

namespace yk {

namespace polyfill {

template<class R, class... Args>
class copyable_function<R(Args...)
                            YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_REF YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_NOEXCEPT> {
private:
  template<class T>
  using is_invocable_using =
#  ifdef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
      is_nothrow_invocable_r<R, T, Args...>
#  else
      is_invocable_r<R, T, Args...>
#  endif
      ;

  template<class VT>
  using is_callable_from = conjunction<
      is_invocable_using<VT YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_REF>,
      is_invocable_using<VT YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_INV_QUALS>>;

  template<class VT>
  static R invoke_target(detail::owning_storage const& storage, Args&&... args) YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_NOEXCEPT
  {
    auto&& f = detail::owning_holder<VT>::get(storage);
    return polyfill::invoke_r<R>(static_cast<VT YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_INV_QUALS>(f), std::forward<Args>(args)...);
  }

  // manage_(op, self, other) destroys the target of self, or relocates or copies it into *other, which has none.
  template<class VT>
  static void manage_target(detail::owning_op op, detail::owning_storage& self, detail::owning_storage* other)
  {
    switch (op) {
      case detail::owning_op::relocate:
        detail::owning_holder<VT>::relocate(self, *other);
        break;
      case detail::owning_op::copy:
        detail::owning_holder<VT>::copy(self, *other);
        break;
      case detail::owning_op::destroy:
        detail::owning_holder<VT>::destroy(self);
        break;
    }
  }

  template<class VT, class... CArgs>
  void construct_target(CArgs&&... args)
  {
    detail::owning_holder<VT>::create(storage_, std::forward<CArgs>(args)...);
    invoke_ = &invoke_target<VT>;
    manage_ = &manage_target<VT>;
  }

  // Moves the target object of other, if any, into *this, which has none, leaving other with none.
  void take(copyable_function& other) noexcept
  {
    if (other.manage_) other.manage_(detail::owning_op::relocate, other.storage_, &storage_);
    invoke_ = other.invoke_;
    manage_ = other.manage_;
    other.invoke_ = nullptr;
    other.manage_ = nullptr;
  }

public:
  using result_type = R;

  copyable_function() noexcept = default;

  copyable_function(std::nullptr_t) noexcept {}

  copyable_function(copyable_function const& other)
  {
    // copy only reads the target, so it is fine if other is a const object
    if (other.manage_) other.manage_(detail::owning_op::copy, const_cast<detail::owning_storage&>(other.storage_), &storage_);
    invoke_ = other.invoke_;
    manage_ = other.manage_;
  }

  copyable_function(copyable_function&& other) noexcept { take(other); }

  template<
      class F, class VT = typename std::decay<F>::type,
      typename std::enable_if<!std::is_same<typename remove_cvref<F>::type, copyable_function>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!extension::is_specialization_of<typename remove_cvref<F>::type, in_place_type_t>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<is_callable_from<VT>::value, std::nullptr_t>::type = nullptr>
  copyable_function(F&& f)
  {
    static_assert(std::is_constructible<VT, F>::value, "[func.wrap.copy.ctor] mandates is_constructible_v<VT, F>");
    static_assert(std::is_copy_constructible<VT>::value, "[func.wrap.copy.ctor] mandates is_copy_constructible_v<VT>");
    if (detail::owning_is_null<typename remove_cvref<F>::type>(f)) return;
    construct_target<VT>(std::forward<F>(f));
  }

  template<
      class T, class... CArgs, class VT = typename std::decay<T>::type,
      typename std::enable_if<conjunction<std::is_constructible<VT, CArgs...>, is_callable_from<VT>>::value, std::nullptr_t>::type = nullptr>
  explicit copyable_function(in_place_type_t<T>, CArgs&&... args)
  {
    static_assert(std::is_same<VT, T>::value, "[func.wrap.copy.ctor] mandates that VT is the same type as T");
    static_assert(std::is_copy_constructible<VT>::value, "[func.wrap.copy.ctor] mandates is_copy_constructible_v<VT>");
    construct_target<VT>(std::forward<CArgs>(args)...);
  }

  template<
      class T, class U, class... CArgs, class VT = typename std::decay<T>::type,
      typename std::enable_if<conjunction<std::is_constructible<VT, std::initializer_list<U>&, CArgs...>, is_callable_from<VT>>::value, std::nullptr_t>::type =
          nullptr>
  explicit copyable_function(in_place_type_t<T>, std::initializer_list<U> ilist, CArgs&&... args)
  {
    static_assert(std::is_same<VT, T>::value, "[func.wrap.copy.ctor] mandates that VT is the same type as T");
    static_assert(std::is_copy_constructible<VT>::value, "[func.wrap.copy.ctor] mandates is_copy_constructible_v<VT>");
    construct_target<VT>(ilist, std::forward<CArgs>(args)...);
  }

  copyable_function& operator=(copyable_function const& other)
  {
    copyable_function(other).swap(*this);
    return *this;
  }

  // noexcept is a strengthening allowed by [res.on.exception.handling], as moving a target never throws here.
  copyable_function& operator=(copyable_function&& other) noexcept
  {
    copyable_function(std::move(other)).swap(*this);
    return *this;
  }

  copyable_function& operator=(std::nullptr_t) noexcept
  {
    if (manage_) manage_(detail::owning_op::destroy, storage_, nullptr);
    invoke_ = nullptr;
    manage_ = nullptr;
    return *this;
  }

  // Constrained, unlike in [func.wrap.copy.ctor] but as in libstdc++, so that is_assignable agrees with is_constructible.
  template<class F, typename std::enable_if<std::is_constructible<copyable_function, F>::value, std::nullptr_t>::type = nullptr>
  copyable_function& operator=(F&& f)
  {
    copyable_function(std::forward<F>(f)).swap(*this);
    return *this;
  }

  ~copyable_function()
  {
    if (manage_) manage_(detail::owning_op::destroy, storage_, nullptr);
  }

  YK_POLYFILL_NODISCARD explicit operator bool() const noexcept { return invoke_ != nullptr; }

  R operator()(Args... args) YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_REF YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_NOEXCEPT
  {
    return invoke_(storage_, std::forward<Args>(args)...);
  }

  void swap(copyable_function& other) noexcept
  {
    copyable_function tmp(std::move(other));
    other.take(*this);
    take(tmp);
  }

  friend void swap(copyable_function& f1, copyable_function& f2) noexcept { f1.swap(f2); }

  YK_POLYFILL_NODISCARD friend bool operator==(copyable_function const& f, std::nullptr_t) noexcept { return !f; }

#  if !(__cpp_impl_three_way_comparison >= 201907L)
  // Since C++20, these are rewritten from the operator== above.
  YK_POLYFILL_NODISCARD friend bool operator==(std::nullptr_t, copyable_function const& f) noexcept { return !f; }
  YK_POLYFILL_NODISCARD friend bool operator!=(copyable_function const& f, std::nullptr_t) noexcept { return static_cast<bool>(f); }
  YK_POLYFILL_NODISCARD friend bool operator!=(std::nullptr_t, copyable_function const& f) noexcept { return static_cast<bool>(f); }
#  endif

private:
  detail::owning_storage storage_ = {nullptr};
  R (*invoke_)(detail::owning_storage const&, Args&&...) YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_NOEXCEPT = nullptr;
  void (*manage_)(detail::owning_op, detail::owning_storage&, detail::owning_storage*) = nullptr;
};

}  // namespace polyfill

}  // namespace yk

#  undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_CV
#  undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_REF
#  undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_INV_QUALS
#  undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_CF_NOEXCEPT

#  undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#  undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#  undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#  undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#endif
