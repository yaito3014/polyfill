#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/span.hpp>

#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace pf = yk::polyfill;

namespace {

struct Base {};
struct Derived : Base {};

// A user-defined contiguous sized range.
struct Buffer {
  int elems[3];
  int* begin() noexcept { return elems; }
  int* end() noexcept { return elems + 3; }
  int* data() noexcept { return elems; }
  std::size_t size() const noexcept { return 3; }
};

constexpr int global_array[] = {1, 2, 3, 4};

}  // namespace

TEST_CASE("span - dynamic_extent")
{
  STATIC_REQUIRE(std::is_same<decltype(pf::dynamic_extent), std::size_t const>::value);
  STATIC_REQUIRE(pf::dynamic_extent == static_cast<std::size_t>(-1));
  STATIC_REQUIRE(pf::span<int>::extent == pf::dynamic_extent);
  STATIC_REQUIRE(pf::span<int, 3>::extent == 3);
}

TEST_CASE("span - member types")
{
  using S = pf::span<int const, 2>;
  STATIC_REQUIRE(std::is_same<S::element_type, int const>::value);
  STATIC_REQUIRE(std::is_same<S::value_type, int>::value);
  STATIC_REQUIRE(std::is_same<S::size_type, std::size_t>::value);
  STATIC_REQUIRE(std::is_same<S::difference_type, std::ptrdiff_t>::value);
  STATIC_REQUIRE(std::is_same<S::pointer, int const*>::value);
  STATIC_REQUIRE(std::is_same<S::const_pointer, int const*>::value);
  STATIC_REQUIRE(std::is_same<S::reference, int const&>::value);
  STATIC_REQUIRE(std::is_same<S::const_reference, int const&>::value);
  STATIC_REQUIRE(std::is_same<std::iterator_traits<S::iterator>::value_type, int>::value);
  STATIC_REQUIRE(std::is_same<std::iterator_traits<S::iterator>::reference, int const&>::value);
  STATIC_REQUIRE(std::is_same<S::reverse_iterator, std::reverse_iterator<S::iterator>>::value);
}

TEST_CASE("span - trivially copyable")
{
  STATIC_REQUIRE(std::is_trivially_copyable<pf::span<int>>::value);
  STATIC_REQUIRE(std::is_trivially_copyable<pf::span<int, 3>>::value);
  STATIC_REQUIRE(sizeof(pf::span<int, 3>) == sizeof(int*));
}

TEST_CASE("span - default construction")
{
  STATIC_REQUIRE(std::is_nothrow_default_constructible<pf::span<int>>::value);
  STATIC_REQUIRE(std::is_nothrow_default_constructible<pf::span<int, 0>>::value);
  STATIC_REQUIRE(!std::is_default_constructible<pf::span<int, 1>>::value);

  constexpr pf::span<int> s;
  STATIC_REQUIRE(s.size() == 0);
  STATIC_REQUIRE(s.data() == nullptr);
  STATIC_REQUIRE(s.empty());

  constexpr pf::span<int, 0> z;
  STATIC_REQUIRE(z.data() == nullptr);
}

TEST_CASE("span - construction from pointer and count")
{
  int arr[] = {1, 2, 3};

  pf::span<int> s(arr, 2);
  CHECK(s.data() == arr);
  CHECK(s.size() == 2);

  pf::span<int, 3> f(arr, 3);
  CHECK(f.data() == arr);
  CHECK(f.size() == 3);

  STATIC_REQUIRE(!std::is_convertible<int*, pf::span<int>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::span<int const>, int*, std::size_t>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int>, int const*, std::size_t>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<Base>, Derived*, std::size_t>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<long>, int*, std::size_t>::value);

  constexpr pf::span<int const> c(global_array, 4);
  STATIC_REQUIRE(c.size() == 4);
  STATIC_REQUIRE(c[3] == 4);
}

TEST_CASE("span - construction from iterator pair")
{
  int arr[] = {1, 2, 3};

  pf::span<int> s(arr, arr + 3);
  CHECK(s.data() == arr);
  CHECK(s.size() == 3);

  pf::span<int const> c(arr + 1, static_cast<int const*>(arr + 3));
  CHECK(c.data() == arr + 1);
  CHECK(c.size() == 2);

  pf::span<int, 3> f(arr, arr + 3);
  CHECK(f.size() == 3);

  pf::span<int> from_iterators(s.begin() + 1, s.end());
  CHECK(from_iterators.data() == arr + 1);
  CHECK(from_iterators.size() == 2);

  constexpr pf::span<int const> k(global_array + 1, global_array + 3);
  STATIC_REQUIRE(k.size() == 2);
  STATIC_REQUIRE(k.front() == 2);
}

TEST_CASE("span - explicit for static extent")
{
  // explicit(extent != dynamic_extent) on the (It, count), (It, End), and range constructors
  STATIC_REQUIRE(std::is_constructible<pf::span<int, 3>, int*, std::size_t>::value);
  STATIC_REQUIRE(std::is_constructible<pf::span<int, 3>, int*, int*>::value);
  STATIC_REQUIRE(std::is_constructible<pf::span<int, 3>, std::vector<int>&>::value);
  STATIC_REQUIRE(!std::is_convertible<std::vector<int>&, pf::span<int, 3>>::value);
  STATIC_REQUIRE(std::is_convertible<std::vector<int>&, pf::span<int>>::value);
}

TEST_CASE("span - construction from built-in array")
{
  int arr[] = {1, 2, 3};

  pf::span<int> s = arr;
  CHECK(s.data() == arr);
  CHECK(s.size() == 3);

  pf::span<int, 3> f = arr;
  CHECK(f.data() == arr);

  pf::span<int const> c = arr;
  CHECK(c.size() == 3);

  STATIC_REQUIRE(std::is_nothrow_constructible<pf::span<int>, int (&)[3]>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int, 2>, int (&)[3]>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int>, int const(&)[3]>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<Base>, Derived(&)[3]>::value);

  constexpr pf::span<int const, 4> k = global_array;
  STATIC_REQUIRE(k.back() == 4);
}

TEST_CASE("span - construction from std::array")
{
  std::array<int, 3> arr = {{1, 2, 3}};
  std::array<int, 3> const& carr = arr;

  pf::span<int> s = arr;
  CHECK(s.data() == arr.data());
  CHECK(s.size() == 3);

  pf::span<int const, 3> c = carr;
  CHECK(c.data() == arr.data());

  STATIC_REQUIRE(std::is_nothrow_constructible<pf::span<int, 3>, std::array<int, 3>&>::value);
  STATIC_REQUIRE(std::is_nothrow_constructible<pf::span<int const>, std::array<int, 3> const&>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int>, std::array<int, 3> const&>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int, 2>, std::array<int, 3>&>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int, 2>, std::array<int, 3> const&>::value);
}

TEST_CASE("span - construction from contiguous range")
{
  std::vector<int> v = {1, 2, 3, 4};

  pf::span<int> s = v;
  CHECK(s.data() == v.data());
  CHECK(s.size() == 4);

  pf::span<int, 4> f(v);
  CHECK(f.data() == v.data());

  std::string str = "abc";
  pf::span<char const> cs = str;
  CHECK(cs.size() == 3);
  CHECK(cs[1] == 'b');

  Buffer buf = {{7, 8, 9}};
  pf::span<int> b = buf;
  CHECK(b.data() == buf.elems);
  CHECK(b.size() == 3);

  std::vector<int> const& cv = v;
  STATIC_REQUIRE(!std::is_constructible<pf::span<int>, std::vector<int> const&>::value);
  pf::span<int const> c = cv;
  CHECK(c.size() == 4);

  // An rvalue range is accepted only when the elements are const, since it is not known to be borrowed.
  STATIC_REQUIRE(!std::is_constructible<pf::span<int>, std::vector<int>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::span<int const>, std::vector<int>>::value);

  STATIC_REQUIRE(!std::is_constructible<pf::span<long>, std::vector<int>&>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int>, int>::value);
}

TEST_CASE("span - construction from initializer_list")
{
  // An initializer_list is a contiguous sized range of const elements.
  std::initializer_list<int> il = {1, 2, 3};
  pf::span<int const> s = il;
  CHECK(s.data() == il.begin());
  CHECK(s.size() == 3);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int>, std::initializer_list<int>&>::value);
}

TEST_CASE("span - converting construction")
{
  int arr[] = {1, 2, 3};
  pf::span<int, 3> f = arr;
  pf::span<int> d = arr;

  pf::span<int const> a = f;
  CHECK(a.data() == arr);
  CHECK(a.size() == 3);

  pf::span<int const, 3> b = f;
  CHECK(b.data() == arr);

  pf::span<int, 3> c(d);
  CHECK(c.data() == arr);

  STATIC_REQUIRE(std::is_nothrow_constructible<pf::span<int const>, pf::span<int, 3>>::value);
  STATIC_REQUIRE(std::is_convertible<pf::span<int, 3>, pf::span<int const>>::value);
  STATIC_REQUIRE(std::is_convertible<pf::span<int, 3>, pf::span<int const, 3>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::span<int, 3>, pf::span<int>>::value);
  STATIC_REQUIRE(!std::is_convertible<pf::span<int>, pf::span<int, 3>>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int, 2>, pf::span<int, 3>>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<int>, pf::span<int const>>::value);
  STATIC_REQUIRE(!std::is_constructible<pf::span<Base>, pf::span<Derived>>::value);
}

TEST_CASE("span - copy")
{
  int arr[] = {1, 2, 3};
  pf::span<int> s = arr;
  pf::span<int> t;
  t = s;
  CHECK(t.data() == arr);
  CHECK(t.size() == 3);
  STATIC_REQUIRE(std::is_nothrow_copy_assignable<pf::span<int, 3>>::value);
}

TEST_CASE("span - static subviews")
{
  int arr[] = {1, 2, 3, 4, 5};
  pf::span<int> d = arr;
  pf::span<int, 5> f = arr;

  STATIC_REQUIRE(std::is_same<decltype(d.first<2>()), pf::span<int, 2>>::value);
  STATIC_REQUIRE(std::is_same<decltype(d.last<2>()), pf::span<int, 2>>::value);
  STATIC_REQUIRE(std::is_same<decltype(d.subspan<1>()), pf::span<int>>::value);
  STATIC_REQUIRE(std::is_same<decltype(d.subspan<1, 2>()), pf::span<int, 2>>::value);
  STATIC_REQUIRE(std::is_same<decltype(f.subspan<1>()), pf::span<int, 4>>::value);
  STATIC_REQUIRE(std::is_same<decltype(f.subspan<1, 3>()), pf::span<int, 3>>::value);

  CHECK(d.first<2>().data() == arr);
  CHECK(d.last<2>().data() == arr + 3);
  CHECK(d.subspan<1>().data() == arr + 1);
  CHECK(d.subspan<1>().size() == 4);
  CHECK(f.subspan<1, 3>().data() == arr + 1);
  CHECK(f.subspan<5>().empty());
}

TEST_CASE("span - dynamic subviews")
{
  int arr[] = {1, 2, 3, 4, 5};
  pf::span<int, 5> f = arr;

  STATIC_REQUIRE(std::is_same<decltype(f.first(2)), pf::span<int>>::value);
  STATIC_REQUIRE(std::is_same<decltype(f.last(2)), pf::span<int>>::value);
  STATIC_REQUIRE(std::is_same<decltype(f.subspan(1)), pf::span<int>>::value);

  CHECK(f.first(2).data() == arr);
  CHECK(f.first(2).size() == 2);
  CHECK(f.last(2).data() == arr + 3);
  CHECK(f.subspan(1).size() == 4);
  CHECK(f.subspan(1, 2).data() == arr + 1);
  CHECK(f.subspan(1, 2).size() == 2);
  CHECK(f.subspan(1, pf::dynamic_extent).size() == 4);
}

TEST_CASE("span - observers and element access")
{
  long arr[] = {1, 2, 3};
  pf::span<long> s = arr;
  CHECK(s.size() == 3);
  CHECK(s.size_bytes() == 3 * sizeof(long));
  CHECK(!s.empty());
  CHECK(&s[1] == arr + 1);
  CHECK(&s.at(2) == arr + 2);
  CHECK(&s.front() == arr);
  CHECK(&s.back() == arr + 2);
  CHECK_THROWS_AS(s.at(3), std::out_of_range);
  STATIC_REQUIRE(std::is_same<decltype(s[0]), long&>::value);
  STATIC_REQUIRE(std::is_same<decltype(s.at(0)), long&>::value);

  s[0] = 10;
  CHECK(arr[0] == 10);
}

TEST_CASE("span - constexpr observers, element access, and subviews")
{
  constexpr pf::span<int const, 4> s = global_array;
  STATIC_REQUIRE(s.size() == 4);
  STATIC_REQUIRE(s.size_bytes() == 4 * sizeof(int));
  STATIC_REQUIRE(!s.empty());
  STATIC_REQUIRE(s[1] == 2);
  STATIC_REQUIRE(s.at(2) == 3);
  STATIC_REQUIRE(s.front() == 1);
  STATIC_REQUIRE(s.back() == 4);
  STATIC_REQUIRE(s.data() == global_array);
  STATIC_REQUIRE(s.first<2>().back() == 2);
  STATIC_REQUIRE(s.last<3>().front() == 2);
  STATIC_REQUIRE(s.subspan<1, 2>().back() == 3);
  STATIC_REQUIRE(s.subspan<1>().size() == 3);
  STATIC_REQUIRE(s.first(3).back() == 3);
  STATIC_REQUIRE(s.last(1).front() == 4);
  STATIC_REQUIRE(s.subspan(2).front() == 3);
  STATIC_REQUIRE(s.subspan(1, 1).size() == 1);
  STATIC_REQUIRE(pf::span<int const>(s).size() == 4);
}

TEST_CASE("span - iterators")
{
  int arr[] = {1, 2, 3};
  pf::span<int> s = arr;

  CHECK(s.end() - s.begin() == 3);
  CHECK(&*s.begin() == arr);
  CHECK(s.begin()[2] == 3);
  CHECK(*(s.end() - 1) == 3);
  CHECK(*s.rbegin() == 3);
  CHECK(s.rend() - s.rbegin() == 3);
  CHECK(*s.cbegin() == 1);
  CHECK(s.cend() - s.cbegin() == 3);
  CHECK(*s.crbegin() == 3);
  CHECK(s.crend() - s.crbegin() == 3);

  STATIC_REQUIRE(std::is_same<decltype(*s.cbegin()), int const&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*s.crbegin()), int const&>::value);
  STATIC_REQUIRE(std::is_base_of<std::random_access_iterator_tag, std::iterator_traits<pf::span<int>::iterator>::iterator_category>::value);

  int sum = 0;
  for (int& x : s) sum += x;
  CHECK(sum == 6);

  pf::span<int> e;
  CHECK(e.begin() == e.end());
  CHECK(e.cbegin() == e.cend());

  pf::span<int>::iterator it = s.begin();
  ++it;
  it++;
  CHECK(*it == 3);
  --it;
  it--;
  CHECK(it == s.begin());
  it += 2;
  it -= 1;
  CHECK(*it == 2);
  CHECK(it > s.begin());
  CHECK(it >= s.begin());
  CHECK(s.begin() < it);
  CHECK(s.begin() <= it);
  CHECK(it != s.begin());
  CHECK(2 + s.begin() == s.begin() + 2);

  pf::span<int>::const_iterator ci = s.begin();
  CHECK(ci == s.cbegin());
  STATIC_REQUIRE(!std::is_convertible<pf::span<int>::const_iterator, pf::span<int>::iterator>::value);
}
