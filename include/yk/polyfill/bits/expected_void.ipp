#ifndef YK_POLYFILL_INCLUDE_EXPECTED
#warning "Do not include this file directly."
#else

// Included once per cv-qualification of void with YK_POLYFILL_EXPECTED_VOID_CV set to that
// qualification; the standard's single constrained partial specialization (requires is_void_v<T>)
// is not expressible before C++20.

namespace yk {

namespace polyfill {

template<class E>
class expected<void YK_POLYFILL_EXPECTED_VOID_CV, E> : private detail::cond_trivial_smf<detail::expected_void_storage_base<E>, E> {
private:
  using base_type = detail::cond_trivial_smf<detail::expected_void_storage_base<E>, E>;

  static_assert(std::is_object<E>::value && !std::is_array<E>::value && !std::is_const<E>::value && !std::is_volatile<E>::value,
                "E must be a valid unexpected type");

public:
  using value_type = void YK_POLYFILL_EXPECTED_VOID_CV;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  template<class U>
  using rebind = expected<U, error_type>;

  constexpr expected() noexcept : base_type(in_place) {}

  // copy/move constructors are implicit (provided by cond_trivial_smf base)

  template<
      class U, class G,
      typename std::enable_if<std::is_void<U>::value && detail::expected_void_can_convert<U, E, G, G const&>::value && std::is_convertible<G const&, E>::value,
                              std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected(expected<U, G> const& rhs) noexcept(std::is_nothrow_constructible<E, G const&>::value) : base_type()
  {
    if (!rhs.has_value()) {
      this->construct_error(rhs.error());
    }
  }

  template<
      class U, class G,
      typename std::enable_if<std::is_void<U>::value && detail::expected_void_can_convert<U, E, G, G const&>::value && !std::is_convertible<G const&, E>::value,
                              std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit expected(expected<U, G> const& rhs) noexcept(std::is_nothrow_constructible<E, G const&>::value) : base_type()
  {
    if (!rhs.has_value()) {
      this->construct_error(rhs.error());
    }
  }

  template<class U, class G,
           typename std::enable_if<std::is_void<U>::value && detail::expected_void_can_convert<U, E, G, G>::value && std::is_convertible<G, E>::value,
                                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected(expected<U, G>&& rhs) noexcept(std::is_nothrow_constructible<E, G>::value) : base_type()
  {
    if (!rhs.has_value()) {
      this->construct_error(std::move(rhs).error());
    }
  }

  template<class U, class G,
           typename std::enable_if<std::is_void<U>::value && detail::expected_void_can_convert<U, E, G, G>::value && !std::is_convertible<G, E>::value,
                                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit expected(expected<U, G>&& rhs) noexcept(std::is_nothrow_constructible<E, G>::value) : base_type()
  {
    if (!rhs.has_value()) {
      this->construct_error(std::move(rhs).error());
    }
  }

  template<class G,
           typename std::enable_if<std::is_constructible<E, G const&>::value && std::is_convertible<G const&, E>::value, std::nullptr_t>::type = nullptr>
  constexpr expected(unexpected<G> const& e) noexcept(std::is_nothrow_constructible<E, G const&>::value) : base_type(unexpect, e.error())
  {
  }

  template<class G,
           typename std::enable_if<std::is_constructible<E, G const&>::value && !std::is_convertible<G const&, E>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(unexpected<G> const& e) noexcept(std::is_nothrow_constructible<E, G const&>::value) : base_type(unexpect, e.error())
  {
  }

  template<class G, typename std::enable_if<std::is_constructible<E, G>::value && std::is_convertible<G, E>::value, std::nullptr_t>::type = nullptr>
  constexpr expected(unexpected<G>&& e) noexcept(std::is_nothrow_constructible<E, G>::value) : base_type(unexpect, std::move(e).error())
  {
  }

  template<class G, typename std::enable_if<std::is_constructible<E, G>::value && !std::is_convertible<G, E>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(unexpected<G>&& e) noexcept(std::is_nothrow_constructible<E, G>::value) : base_type(unexpect, std::move(e).error())
  {
  }

  constexpr explicit expected(in_place_t) noexcept : base_type(in_place) {}

  template<class... Args, typename std::enable_if<std::is_constructible<E, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value)
      : base_type(unexpect, std::forward<Args>(args)...)
  {
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<E, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
      noexcept(std::is_nothrow_constructible<E, std::initializer_list<U>&, Args...>::value)
      : base_type(unexpect, il, std::forward<Args>(args)...)
  {
  }

  // assignment (copy/move assignment are implicit)

  template<class G,
           typename std::enable_if<std::is_constructible<E, G const&>::value && std::is_assignable<E&, G const&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected& operator=(unexpected<G> const& e)
  {
    if (has_value()) {
      this->construct_error(e.error());
    } else {
      this->get_error() = e.error();
    }
    return *this;
  }

  template<class G, typename std::enable_if<std::is_constructible<E, G>::value && std::is_assignable<E&, G>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected& operator=(unexpected<G>&& e)
  {
    if (has_value()) {
      this->construct_error(std::move(e).error());
    } else {
      this->get_error() = std::move(e).error();
    }
    return *this;
  }

  YK_POLYFILL_CXX20_CONSTEXPR void emplace() noexcept { this->destroy(); }

  // [expected.void.swap]
  template<class E2 = E,
           typename std::enable_if<is_swappable<E2>::value && std::is_move_constructible<E2>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible<E>::value && is_nothrow_swappable<E>::value)
  {
    if (rhs.has_value()) {
      if (!has_value()) {
        rhs.construct_error(std::move(this->get_error()));
        this->destroy();
      }
    } else {
      if (has_value()) {
        this->construct_error(std::move(rhs.get_error()));
        rhs.destroy();
      } else {
        using std::swap;
        swap(this->get_error(), rhs.get_error());
      }
    }
  }

  // observers

  YK_POLYFILL_NODISCARD constexpr explicit operator bool() const noexcept { return has_value(); }
  YK_POLYFILL_NODISCARD constexpr bool has_value() const noexcept { return base_type::has_value(); }

  YK_POLYFILL_CXX14_CONSTEXPR void operator*() const noexcept {}

  YK_POLYFILL_CXX14_CONSTEXPR void value() const&
  {
    static_assert(std::is_copy_constructible<E>::value, "E must be copy constructible");
    if (!has_value()) throw bad_expected_access<E>(this->get_error());
  }
  YK_POLYFILL_CXX14_CONSTEXPR void value() &&
  {
    static_assert(std::is_copy_constructible<E>::value && std::is_move_constructible<E>::value, "E must be copy and move constructible");
    if (!has_value()) throw bad_expected_access<E>(std::move(this->get_error()));
  }

  YK_POLYFILL_NODISCARD constexpr E const& error() const& noexcept { return this->get_error(); }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E& error() & noexcept { return this->get_error(); }
  YK_POLYFILL_NODISCARD constexpr E const&& error() const&& noexcept { return std::move(*this).base_get_error(); }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E&& error() && noexcept { return std::move(*this).base_get_error(); }

  template<class G = E>
  YK_POLYFILL_CXX14_CONSTEXPR E error_or(G&& e) const&
  {
    static_assert(std::is_copy_constructible<E>::value && std::is_convertible<G, E>::value, "E must be copy constructible and the argument convertible to E");
    return has_value() ? static_cast<E>(std::forward<G>(e)) : this->get_error();
  }

  template<class G = E>
  YK_POLYFILL_CXX14_CONSTEXPR E error_or(G&& e) &&
  {
    static_assert(std::is_move_constructible<E>::value && std::is_convertible<G, E>::value, "E must be move constructible and the argument convertible to E");
    return has_value() ? static_cast<E>(std::forward<G>(e)) : std::move(this->get_error());
  }

  // monadic operations

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) & -> typename remove_cvref<typename invoke_result<F>::type>::type
  {
    using U = typename remove_cvref<typename invoke_result<F>::type>::type;
    static_assert(detail::is_expected<U>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename U::error_type, E>::value, "F's expected result must have the same error_type");
    if (has_value()) return polyfill::invoke(std::forward<F>(f));
    return U(unexpect, this->get_error());
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2 const&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const& -> typename remove_cvref<typename invoke_result<F>::type>::type
  {
    using U = typename remove_cvref<typename invoke_result<F>::type>::type;
    static_assert(detail::is_expected<U>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename U::error_type, E>::value, "F's expected result must have the same error_type");
    if (has_value()) return polyfill::invoke(std::forward<F>(f));
    return U(unexpect, this->get_error());
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) && -> typename remove_cvref<typename invoke_result<F>::type>::type
  {
    using U = typename remove_cvref<typename invoke_result<F>::type>::type;
    static_assert(detail::is_expected<U>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename U::error_type, E>::value, "F's expected result must have the same error_type");
    if (has_value()) return polyfill::invoke(std::forward<F>(f));
    return U(unexpect, std::move(this->get_error()));
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2 const&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const&& -> typename remove_cvref<typename invoke_result<F>::type>::type
  {
    using U = typename remove_cvref<typename invoke_result<F>::type>::type;
    static_assert(detail::is_expected<U>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename U::error_type, E>::value, "F's expected result must have the same error_type");
    if (has_value()) return polyfill::invoke(std::forward<F>(f));
    return U(unexpect, std::move(this->get_error()));
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto or_else(F&& f) & -> typename remove_cvref<typename invoke_result<F, E&>::type>::type
  {
    using G = typename remove_cvref<typename invoke_result<F, E&>::type>::type;
    static_assert(detail::is_expected<G>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename G::value_type, void YK_POLYFILL_EXPECTED_VOID_CV>::value, "F's expected result must have the same value_type");
    if (has_value()) return G();
    return polyfill::invoke(std::forward<F>(f), this->get_error());
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto or_else(F&& f) const& -> typename remove_cvref<typename invoke_result<F, E const&>::type>::type
  {
    using G = typename remove_cvref<typename invoke_result<F, E const&>::type>::type;
    static_assert(detail::is_expected<G>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename G::value_type, void YK_POLYFILL_EXPECTED_VOID_CV>::value, "F's expected result must have the same value_type");
    if (has_value()) return G();
    return polyfill::invoke(std::forward<F>(f), this->get_error());
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto or_else(F&& f) && -> typename remove_cvref<typename invoke_result<F, E&&>::type>::type
  {
    using G = typename remove_cvref<typename invoke_result<F, E&&>::type>::type;
    static_assert(detail::is_expected<G>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename G::value_type, void YK_POLYFILL_EXPECTED_VOID_CV>::value, "F's expected result must have the same value_type");
    if (has_value()) return G();
    return polyfill::invoke(std::forward<F>(f), std::move(this->get_error()));
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto or_else(F&& f) const&& -> typename remove_cvref<typename invoke_result<F, E const&&>::type>::type
  {
    using G = typename remove_cvref<typename invoke_result<F, E const&&>::type>::type;
    static_assert(detail::is_expected<G>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename G::value_type, void YK_POLYFILL_EXPECTED_VOID_CV>::value, "F's expected result must have the same value_type");
    if (has_value()) return G();
    return polyfill::invoke(std::forward<F>(f), std::move(this->get_error()));
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) & -> expected<typename std::remove_cv<typename invoke_result<F>::type>::type, E>
  {
    using U = typename std::remove_cv<typename invoke_result<F>::type>::type;
    if (has_value()) return detail::expected_transform_make<expected<U, E>>(bool_constant<std::is_void<U>::value>{}, std::forward<F>(f));
    return expected<U, E>(unexpect, this->get_error());
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2 const&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) const& -> expected<typename std::remove_cv<typename invoke_result<F>::type>::type, E>
  {
    using U = typename std::remove_cv<typename invoke_result<F>::type>::type;
    if (has_value()) return detail::expected_transform_make<expected<U, E>>(bool_constant<std::is_void<U>::value>{}, std::forward<F>(f));
    return expected<U, E>(unexpect, this->get_error());
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) && -> expected<typename std::remove_cv<typename invoke_result<F>::type>::type, E>
  {
    using U = typename std::remove_cv<typename invoke_result<F>::type>::type;
    if (has_value()) return detail::expected_transform_make<expected<U, E>>(bool_constant<std::is_void<U>::value>{}, std::forward<F>(f));
    return expected<U, E>(unexpect, std::move(this->get_error()));
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2 const&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) const&& -> expected<typename std::remove_cv<typename invoke_result<F>::type>::type, E>
  {
    using U = typename std::remove_cv<typename invoke_result<F>::type>::type;
    if (has_value()) return detail::expected_transform_make<expected<U, E>>(bool_constant<std::is_void<U>::value>{}, std::forward<F>(f));
    return expected<U, E>(unexpect, std::move(this->get_error()));
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform_error(F&& f) &
      -> expected<void YK_POLYFILL_EXPECTED_VOID_CV, typename std::remove_cv<typename invoke_result<F, E&>::type>::type>
  {
    using G = typename std::remove_cv<typename invoke_result<F, E&>::type>::type;
    if (has_value()) return expected<void YK_POLYFILL_EXPECTED_VOID_CV, G>();
    return expected<void YK_POLYFILL_EXPECTED_VOID_CV, G>(unexpect, polyfill::invoke(std::forward<F>(f), this->get_error()));
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform_error(F&& f) const&
      -> expected<void YK_POLYFILL_EXPECTED_VOID_CV, typename std::remove_cv<typename invoke_result<F, E const&>::type>::type>
  {
    using G = typename std::remove_cv<typename invoke_result<F, E const&>::type>::type;
    if (has_value()) return expected<void YK_POLYFILL_EXPECTED_VOID_CV, G>();
    return expected<void YK_POLYFILL_EXPECTED_VOID_CV, G>(unexpect, polyfill::invoke(std::forward<F>(f), this->get_error()));
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform_error(F&& f) &&
      -> expected<void YK_POLYFILL_EXPECTED_VOID_CV, typename std::remove_cv<typename invoke_result<F, E&&>::type>::type>
  {
    using G = typename std::remove_cv<typename invoke_result<F, E&&>::type>::type;
    if (has_value()) return expected<void YK_POLYFILL_EXPECTED_VOID_CV, G>();
    return expected<void YK_POLYFILL_EXPECTED_VOID_CV, G>(unexpect, polyfill::invoke(std::forward<F>(f), std::move(this->get_error())));
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform_error(F&& f) const&&
      -> expected<void YK_POLYFILL_EXPECTED_VOID_CV, typename std::remove_cv<typename invoke_result<F, E const&&>::type>::type>
  {
    using G = typename std::remove_cv<typename invoke_result<F, E const&&>::type>::type;
    if (has_value()) return expected<void YK_POLYFILL_EXPECTED_VOID_CV, G>();
    return expected<void YK_POLYFILL_EXPECTED_VOID_CV, G>(unexpect, polyfill::invoke(std::forward<F>(f), std::move(this->get_error())));
  }

private:
  YK_POLYFILL_CXX14_CONSTEXPR E&& base_get_error() && noexcept { return static_cast<base_type&&>(*this).get_error(); }
  constexpr E const&& base_get_error() const&& noexcept { return static_cast<base_type const&&>(*this).get_error(); }
};

}  // namespace polyfill

}  // namespace yk

#endif
