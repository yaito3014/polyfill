#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/new.hpp>

#include <new>
#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

namespace {

struct Widget {
  int value;
  explicit Widget(int v) : value(v) {}
};

constexpr int constant = 42;

// The example of [basic.life]: each call replaces *this with an object of the other type.
struct Transmogrifiable {
  virtual int transmogrify();
};

struct Transmogrified : Transmogrifiable {
  int transmogrify() override
  {
    ::new (static_cast<void*>(this)) Transmogrifiable;
    return 2;
  }
};

int Transmogrifiable::transmogrify()
{
  ::new (static_cast<void*>(this)) Transmogrified;
  return 1;
}

static_assert(sizeof(Transmogrified) == sizeof(Transmogrifiable), "the replacement must fit in the same storage");

}  // namespace

TEST_CASE("launder")
{
  STATIC_REQUIRE(std::is_same<decltype(pf::launder(std::declval<int*>())), int*>::value);
  STATIC_REQUIRE(std::is_same<decltype(pf::launder(std::declval<int const*>())), int const*>::value);
  STATIC_REQUIRE(noexcept(pf::launder(std::declval<int*>())));

  // [ptr.launder]: usable in a core constant expression when its argument could be used in its place
  STATIC_REQUIRE(pf::launder(&constant) == &constant);
  STATIC_REQUIRE(*pf::launder(&constant) == 42);

  {
    // the result points to an object created in storage that another pointer points to the beginning of
    alignas(Widget) unsigned char storage[sizeof(Widget)];
    Widget* const created = ::new (static_cast<void*>(storage)) Widget(42);
    Widget* const laundered = pf::launder(reinterpret_cast<Widget*>(storage));
    CHECK(laundered == created);
    CHECK(laundered->value == 42);
    laundered->~Widget();
  }

  {
    // the result reaches an object of another type that replaced the one the argument points to
    Transmogrifiable a;
    CHECK(a.transmogrify() == 1);
    CHECK(pf::launder(&a)->transmogrify() == 2);
  }
}
