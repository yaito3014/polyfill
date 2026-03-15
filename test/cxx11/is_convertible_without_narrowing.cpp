#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/extension/is_convertible_without_narrowing.hpp>

#include <cstddef>
#include <cstdint>

namespace pf = yk::polyfill;

enum class conversion_kind {
  without_narrowing,
  narrowing,
  not_convertible,
};

template<
    class From, class To, bool IsConvertible = std::is_convertible<From, To>::value,
    bool IsConvertibleWithoutNarrowing = pf::extension::is_convertible_without_narrowing<From, To>::value>
struct test_convertible_without_narrowing {};

template<class From, class To>
struct test_convertible_without_narrowing<From, To, /*IsConvertible = */ true, /* IsConvertibleWithoutNarrowing = */ true> {
  static constexpr conversion_kind value = conversion_kind::without_narrowing;
};

template<class From, class To>
struct test_convertible_without_narrowing<From, To, /*IsConvertible = */ true, /* IsConvertibleWithoutNarrowing = */ false> {
  static constexpr conversion_kind value = conversion_kind::narrowing;
};

template<class From, class To>
struct test_convertible_without_narrowing<From, To, /*IsConvertible = */ false, /* IsConvertibleWithoutNarrowing = */ false> {
  static constexpr conversion_kind value = conversion_kind::not_convertible;
};

// corrupted version: if IsConvertible is false, IsConvertibleWithoutNarrowing must be false
template<class From, class To>
struct test_convertible_without_narrowing<From, To, /*IsConvertible = */ false, /* IsConvertibleWithoutNarrowing = */ true> {};

TEST_CASE("is_convertible_without_narrowing")
{
  // conversion between integer types
  STATIC_REQUIRE(test_convertible_without_narrowing<std::int16_t, std::int32_t>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<std::int32_t, std::int16_t>::value == conversion_kind::narrowing);

  // conversion between floating-point types
  STATIC_REQUIRE(test_convertible_without_narrowing<float, double>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<double, float>::value == conversion_kind::narrowing);

  // conversion between integer and floating-point types
  STATIC_REQUIRE(test_convertible_without_narrowing<int, double>::value == conversion_kind::narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<double, int>::value == conversion_kind::narrowing);

  // conversion including void
  STATIC_REQUIRE(test_convertible_without_narrowing<int, void>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<void, int>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<void, void>::value == conversion_kind::without_narrowing);

  // conversion including function type
  STATIC_REQUIRE(test_convertible_without_narrowing<void(void), void (*)(void)>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<void (*)(void), void(void)>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<void(void), void(void)>::value == conversion_kind::not_convertible);

  // conversion including reference type
  STATIC_REQUIRE(test_convertible_without_narrowing<int, int&>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<int&, int>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int&, int&>::value == conversion_kind::without_narrowing);

  STATIC_REQUIRE(test_convertible_without_narrowing<int, int const&>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int const&, int>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int&, int const&>::value == conversion_kind::without_narrowing);

  STATIC_REQUIRE(test_convertible_without_narrowing<int const, int const&>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int const&, int const>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int const&, int const&>::value == conversion_kind::without_narrowing);

  STATIC_REQUIRE(test_convertible_without_narrowing<int const, int&>::value == conversion_kind::not_convertible);   // lose constness
  STATIC_REQUIRE(test_convertible_without_narrowing<int const&, int&>::value == conversion_kind::not_convertible);  // lose constness

  STATIC_REQUIRE(test_convertible_without_narrowing<int&, double>::value == conversion_kind::narrowing);

  // conversion including unbounded array type
  STATIC_REQUIRE(test_convertible_without_narrowing<int[], int*>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int*, int[]>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<int[], int[]>::value == conversion_kind::not_convertible);

  STATIC_REQUIRE(test_convertible_without_narrowing<int[], int const*>::value == conversion_kind::without_narrowing);

  STATIC_REQUIRE(test_convertible_without_narrowing<int[], int (&)[]>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<int[], int const(&)[]>::value == conversion_kind::without_narrowing);

  // conversion including bounded array type
  STATIC_REQUIRE(test_convertible_without_narrowing<int[3], int*>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int*, int[3]>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<int[3], int[3]>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<int[3], int const*>::value == conversion_kind::without_narrowing);

  // narrowing through reference (temporary created)
  STATIC_REQUIRE(test_convertible_without_narrowing<double, int const&>::value == conversion_kind::narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int, double const&>::value == conversion_kind::narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<double, int&&>::value == conversion_kind::narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<std::int32_t, std::int16_t const&>::value == conversion_kind::narrowing);

  // non-narrowing through reference (temporary created)
  STATIC_REQUIRE(test_convertible_without_narrowing<std::int16_t, std::int32_t const&>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<float, double const&>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<std::int16_t, std::int32_t&&>::value == conversion_kind::without_narrowing);

  // non-const lvalue reference: temporary can't bind, is_convertible handles it
  STATIC_REQUIRE(test_convertible_without_narrowing<double, int&>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<std::int16_t, std::int32_t&>::value == conversion_kind::not_convertible);

  // conversion including function reference type
  STATIC_REQUIRE(test_convertible_without_narrowing<void(), void (&)()>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<void (*)(), void (&)()>::value == conversion_kind::not_convertible);

  // conversion including abstract class reference type
  struct Abstract {
    virtual void f() = 0;
  };
  struct Concrete : Abstract {
    void f() override {}
  };
  STATIC_REQUIRE(test_convertible_without_narrowing<Concrete&, Abstract&>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<Concrete&, Abstract const&>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<Concrete const&, Abstract&>::value == conversion_kind::not_convertible);  // lose constness
  STATIC_REQUIRE(test_convertible_without_narrowing<Concrete const&, Abstract const&>::value == conversion_kind::without_narrowing);

  // conversion including cv-qualified void
  STATIC_REQUIRE(test_convertible_without_narrowing<void const, void>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<void, void const>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<void const, void const>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<void volatile, void const volatile>::value == conversion_kind::without_narrowing);

  // same type, different cv-qualification (non-reference)
  STATIC_REQUIRE(test_convertible_without_narrowing<int, int const>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int const, int>::value == conversion_kind::without_narrowing);

  // bool conversion
  STATIC_REQUIRE(test_convertible_without_narrowing<bool, int>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int, bool>::value == conversion_kind::narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int*, bool>::value == conversion_kind::narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<void (*)(void), bool>::value == conversion_kind::narrowing);
// MSVC's implementation differs across versions, so the behavior cannot be reliably tested.
#if !defined(_MSC_VER)
  // nullptr_t -> bool: not convertible (CWG 1781)
  STATIC_REQUIRE(test_convertible_without_narrowing<std::nullptr_t, bool>::value == conversion_kind::not_convertible);
#endif
  // conversion including enum type
  enum UnscopedEnum : int { ue_value = 0 };
  enum class ScopedEnum : int { se_value = 0 };
  STATIC_REQUIRE(test_convertible_without_narrowing<UnscopedEnum, int>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<UnscopedEnum, long long>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<UnscopedEnum, float>::value == conversion_kind::narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<ScopedEnum, int>::value == conversion_kind::not_convertible);

  // conversion between signed and unsigned integer types
  STATIC_REQUIRE(test_convertible_without_narrowing<unsigned int, int>::value == conversion_kind::narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int, unsigned int>::value == conversion_kind::narrowing);

  // conversion including char type
  STATIC_REQUIRE(test_convertible_without_narrowing<char, int>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int, char>::value == conversion_kind::narrowing);

  // conversion including nullptr_t
  STATIC_REQUIRE(test_convertible_without_narrowing<std::nullptr_t, int*>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<std::nullptr_t, void*>::value == conversion_kind::without_narrowing);

  // conversion including pointer cv-qualification
  STATIC_REQUIRE(test_convertible_without_narrowing<int*, int const*>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<int const*, int*>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<int*, void*>::value == conversion_kind::without_narrowing);

  // conversion including derived-to-base
  struct Base {};
  struct Derived : Base {};
  STATIC_REQUIRE(test_convertible_without_narrowing<Derived*, Base*>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<Base*, Derived*>::value == conversion_kind::not_convertible);
  STATIC_REQUIRE(test_convertible_without_narrowing<Derived&, Base&>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<Derived, Base>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<Derived, Base const&>::value == conversion_kind::without_narrowing);

  // conversion including user-defined implicit conversion
  struct ImplicitToInt {
    operator int();
  };
  STATIC_REQUIRE(test_convertible_without_narrowing<ImplicitToInt, int>::value == conversion_kind::without_narrowing);
  STATIC_REQUIRE(test_convertible_without_narrowing<ImplicitToInt, long long>::value == conversion_kind::without_narrowing);

  struct ImplicitFromInt {
    ImplicitFromInt(int);
  };
  STATIC_REQUIRE(test_convertible_without_narrowing<int, ImplicitFromInt>::value == conversion_kind::without_narrowing);

  struct ExplicitFromInt {
    explicit ExplicitFromInt(int);
  };
  STATIC_REQUIRE(test_convertible_without_narrowing<int, ExplicitFromInt>::value == conversion_kind::not_convertible);
}
