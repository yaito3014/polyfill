#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

struct NothrowConvertibleFromInt {
  NothrowConvertibleFromInt(int) noexcept {}
};

struct ThrowingConvertibleFromInt {
  ThrowingConvertibleFromInt(int) noexcept(false) {}
};

struct ExplicitOnly {
  explicit ExplicitOnly(int) noexcept {}
};

struct NotConvertible {};

struct ThrowingDestructor {
  ThrowingDestructor(int) noexcept {}
  ~ThrowingDestructor() noexcept(false) {}
};

struct NothrowConvertibleFromIntPtr {
  NothrowConvertibleFromIntPtr(int*) noexcept {}
};

struct ThrowingConvertibleFromIntPtr {
  ThrowingConvertibleFromIntPtr(int*) noexcept(false) {}
};

namespace pf = yk::polyfill;

TEST_CASE("is_nothrow_convertible")
{
  // built-in types
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, int>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, double>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, long>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<double, int>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int*, void*>::value == true);

  // nothrow implicit conversion
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, NothrowConvertibleFromInt>::value == true);

  // throwing implicit conversion
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, ThrowingConvertibleFromInt>::value == false);

  // potentially-throwing destructor of To counts, as in every major implementation (LWG 3400)
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, ThrowingDestructor>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<ThrowingDestructor&, ThrowingDestructor&>::value == true);

  // explicit-only (not implicitly convertible)
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, ExplicitOnly>::value == false);

  // not convertible at all
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, NotConvertible>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<NotConvertible, int>::value == false);

  // void
  STATIC_REQUIRE(pf::is_nothrow_convertible<void, void>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<void const, void>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<void, void const volatile>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<void const volatile, void volatile>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int, void>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<void, int>::value == false);

  // array types (not returnable, so not convertible)
  STATIC_REQUIRE(pf::is_nothrow_convertible<int*, int[]>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int*, int[5]>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], int[]>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], int[5]>::value == false);

  // arrays of unknown bound as From (declval yields an xvalue array, which decays)
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], int*>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], int const*>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], int>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], void>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], NothrowConvertibleFromIntPtr>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], ThrowingConvertibleFromIntPtr>::value == false);

  // references to arrays of unknown bound
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], int (&&)[]>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], int (&)[]>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int[], int const(&)[]>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int (&)[], int (&)[]>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int (&)[], int (&&)[]>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int (&)[], int*>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_convertible<int*, int (&)[]>::value == false);

  // function types (not returnable, so not convertible)
  STATIC_REQUIRE(pf::is_nothrow_convertible<void (*)(), void()>::value == false);
}
