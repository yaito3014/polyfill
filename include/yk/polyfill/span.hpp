#ifndef YK_ZZ_POLYFILL_SPAN_HPP
#define YK_ZZ_POLYFILL_SPAN_HPP

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/type_traits.hpp>

#include <yk/polyfill/config.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#if __cpp_lib_ranges >= 201911L
#  include <ranges>
#  define YK_POLYFILL_DETAIL_SPAN_USE_RANGES
#endif

#if YK_POLYFILL_CXX_VERSION >= 202002L
#  include <compare>
#endif

namespace yk {

namespace polyfill {

YK_POLYFILL_INLINE constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

template<class ElementType, std::size_t Extent = dynamic_extent>
class span;

namespace detail {

template<class T>
class span_iterator {
public:
#ifdef YK_POLYFILL_DETAIL_SPAN_USE_RANGES
  using iterator_concept = std::contiguous_iterator_tag;
#endif
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = typename std::remove_cv<T>::type;
  using pointer = T*;
  using reference = T&;

  constexpr span_iterator() noexcept = default;

  // iterator to const_iterator
  template<class U, typename std::enable_if<!std::is_same<U, T>::value && std::is_convertible<U (*)[], T (*)[]>::value, std::nullptr_t>::type = nullptr>
  constexpr span_iterator(span_iterator<U> const& other) noexcept : ptr_(other.operator->())
  {
  }

  constexpr reference operator*() const noexcept { return *ptr_; }

  constexpr pointer operator->() const noexcept { return ptr_; }

  constexpr reference operator[](difference_type n) const noexcept { return ptr_[n]; }

  YK_POLYFILL_CXX14_CONSTEXPR span_iterator& operator++() noexcept
  {
    ++ptr_;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR span_iterator operator++(int) noexcept
  {
    span_iterator temporary = *this;
    ++*this;
    return temporary;
  }

  YK_POLYFILL_CXX14_CONSTEXPR span_iterator& operator--() noexcept
  {
    --ptr_;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR span_iterator operator--(int) noexcept
  {
    span_iterator temporary = *this;
    --*this;
    return temporary;
  }

  YK_POLYFILL_CXX14_CONSTEXPR span_iterator& operator+=(difference_type n) noexcept
  {
    ptr_ += n;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR span_iterator& operator-=(difference_type n) noexcept
  {
    ptr_ -= n;
    return *this;
  }

  friend constexpr span_iterator operator+(span_iterator const& it, difference_type n) noexcept { return span_iterator(it.ptr_ + n); }

  friend constexpr span_iterator operator+(difference_type n, span_iterator const& it) noexcept { return span_iterator(it.ptr_ + n); }

  friend constexpr span_iterator operator-(span_iterator const& it, difference_type n) noexcept { return span_iterator(it.ptr_ - n); }

  friend constexpr difference_type operator-(span_iterator const& a, span_iterator const& b) noexcept { return a.ptr_ - b.ptr_; }

  friend constexpr bool operator==(span_iterator const& a, span_iterator const& b) noexcept { return a.ptr_ == b.ptr_; }

  friend constexpr bool operator!=(span_iterator const& a, span_iterator const& b) noexcept { return a.ptr_ != b.ptr_; }

  friend constexpr bool operator<(span_iterator const& a, span_iterator const& b) noexcept { return a.ptr_ < b.ptr_; }

  friend constexpr bool operator<=(span_iterator const& a, span_iterator const& b) noexcept { return a.ptr_ <= b.ptr_; }

  friend constexpr bool operator>(span_iterator const& a, span_iterator const& b) noexcept { return a.ptr_ > b.ptr_; }

  friend constexpr bool operator>=(span_iterator const& a, span_iterator const& b) noexcept { return a.ptr_ >= b.ptr_; }

#if YK_POLYFILL_CXX_VERSION >= 202002L
  friend constexpr std::strong_ordering operator<=>(span_iterator const& a, span_iterator const& b) noexcept { return a.ptr_ <=> b.ptr_; }
#endif

private:
  template<class, std::size_t>
  friend class polyfill::span;

  constexpr explicit span_iterator(pointer ptr) noexcept : ptr_(ptr) {}

  pointer ptr_ = nullptr;
};

// Holds the size only for a dynamic extent, so that a span of static extent is one pointer.
template<std::size_t Extent>
class span_extent {
public:
  constexpr explicit span_extent(std::size_t) noexcept {}

  constexpr std::size_t stored_size() const noexcept { return Extent; }
};

template<>
class span_extent<dynamic_extent> {
public:
  constexpr explicit span_extent(std::size_t size) noexcept : size_(size) {}

  constexpr std::size_t stored_size() const noexcept { return size_; }

private:
  std::size_t size_;
};

template<class T>
struct is_span : false_type {};

template<class T, std::size_t Extent>
struct is_span<span<T, Extent>> : true_type {};

template<class T>
struct is_std_array : false_type {};

template<class T, std::size_t N>
struct is_std_array<std::array<T, N>> : true_type {};

// Only qualification conversions of the element type are allowed.
template<class From, class To>
struct is_span_element_convertible : std::is_convertible<From (*)[], To (*)[]> {};

template<class It>
using span_iter_reference_t = decltype(*std::declval<It&>());

#ifdef YK_POLYFILL_DETAIL_SPAN_USE_RANGES

template<class It>
struct is_span_contiguous_iterator : bool_constant<std::contiguous_iterator<It>> {};

template<class It>
constexpr auto span_to_address(It const& it) noexcept(noexcept(std::to_address(it))) -> decltype(std::to_address(it))
{
  return std::to_address(it);
}

template<class End, class It>
struct is_span_sized_sentinel_for : bool_constant<std::sized_sentinel_for<End, It>> {};

#else

// contiguous_iterator cannot be detected before C++20, so only the iterators known to be contiguous are accepted.
template<class It>
struct is_span_contiguous_iterator : bool_constant<std::is_pointer<It>::value && std::is_object<typename std::remove_pointer<It>::type>::value> {};

template<class T>
struct is_span_contiguous_iterator<span_iterator<T>> : true_type {};

template<class T>
constexpr T* span_to_address(T* p) noexcept
{
  return p;
}

template<class T>
constexpr T* span_to_address(span_iterator<T> const& it) noexcept
{
  return it.operator->();
}

template<class End, class It, class = void>
struct is_span_sized_sentinel_for : false_type {};

template<class End, class It>
struct is_span_sized_sentinel_for<
    End, It,
    void_t<
        decltype(std::declval<It const&>() == std::declval<End const&>()), decltype(std::declval<End const&>() - std::declval<It const&>()),
        decltype(std::declval<It const&>() - std::declval<End const&>())>>
    : bool_constant<
          std::is_convertible<decltype(std::declval<It const&>() == std::declval<End const&>()), bool>::value
          && std::is_same<decltype(std::declval<End const&>() - std::declval<It const&>()), std::ptrdiff_t>::value
          && std::is_same<decltype(std::declval<It const&>() - std::declval<End const&>()), std::ptrdiff_t>::value> {};

#endif

template<class It, class ElementType, class = void>
struct is_span_compatible_iterator : false_type {};

template<class It, class ElementType>
struct is_span_compatible_iterator<It, ElementType, typename std::enable_if<is_span_contiguous_iterator<It>::value>::type>
    : is_span_element_convertible<typename std::remove_reference<span_iter_reference_t<It>>::type, ElementType> {};

template<class End, class It, class ElementType>
struct is_span_compatible_sentinel
    : conjunction<is_span_compatible_iterator<It, ElementType>, is_span_sized_sentinel_for<End, It>, negation<std::is_convertible<End, std::size_t>>> {};

template<class R>
struct is_span_range_excluded
    : disjunction<is_span<typename remove_cvref<R>::type>, is_std_array<typename remove_cvref<R>::type>, std::is_array<typename remove_cvref<R>::type>> {};

#ifdef YK_POLYFILL_DETAIL_SPAN_USE_RANGES

template<class R>
struct is_span_contiguous_range : bool_constant<std::ranges::contiguous_range<R>> {};

template<class R, class ElementType, class = void>
struct is_span_compatible_range_impl : false_type {};

template<class R, class ElementType>
struct is_span_compatible_range_impl<R, ElementType, typename std::enable_if<std::ranges::contiguous_range<R> && std::ranges::sized_range<R>>::type>
    : bool_constant<
          (std::ranges::borrowed_range<R> || std::is_const<ElementType>::value)
          && is_span_element_convertible<std::remove_reference_t<std::ranges::range_reference_t<R>>, ElementType>::value> {};

template<class R>
using span_range_element_t = std::remove_reference_t<std::ranges::range_reference_t<R>>;

template<class R>
constexpr auto span_range_data(R& r) -> decltype(std::ranges::data(r))
{
  return std::ranges::data(r);
}

template<class R>
constexpr std::size_t span_range_size(R& r)
{
  return static_cast<std::size_t>(std::ranges::size(r));
}

#else

// ranges::data and ranges::size are approximated by member data() (or begin(), when it is a pointer) and member size().

template<class R, class = void>
struct has_span_member_data : false_type {};

template<class R>
struct has_span_member_data<R, void_t<decltype(std::declval<R&>().data())>>
    : bool_constant<
          std::is_pointer<decltype(std::declval<R&>().data())>::value
          && std::is_object<typename std::remove_pointer<decltype(std::declval<R&>().data())>::type>::value> {};

template<class R, class = void>
struct has_span_member_begin : false_type {};

template<class R>
struct has_span_member_begin<R, void_t<decltype(std::declval<R&>().begin())>>
    : bool_constant<
          std::is_pointer<decltype(std::declval<R&>().begin())>::value
          && std::is_object<typename std::remove_pointer<decltype(std::declval<R&>().begin())>::type>::value> {};

template<class R, class = void>
struct has_span_member_size : false_type {};

template<class R>
struct has_span_member_size<R, void_t<decltype(std::declval<R&>().size())>> : bool_constant<std::is_integral<decltype(std::declval<R&>().size())>::value> {};

template<class R>
struct is_span_contiguous_range : disjunction<has_span_member_data<R>, has_span_member_begin<R>> {};

template<class R, typename std::enable_if<has_span_member_data<R>::value, std::nullptr_t>::type = nullptr>
constexpr auto span_range_data(R& r) -> decltype(r.data())
{
  return r.data();
}

template<class R, typename std::enable_if<!has_span_member_data<R>::value && has_span_member_begin<R>::value, std::nullptr_t>::type = nullptr>
constexpr auto span_range_data(R& r) -> decltype(r.begin())
{
  return r.begin();
}

template<class R>
using span_range_element_t = typename std::remove_pointer<decltype(detail::span_range_data(std::declval<R&>()))>::type;

template<class R>
constexpr std::size_t span_range_size(R& r)
{
  return static_cast<std::size_t>(r.size());
}

template<class R, class ElementType, class = void>
struct is_span_compatible_range_impl : false_type {};

// An lvalue is a borrowed range; an rvalue cannot be known to be one without ranges::enable_borrowed_range.
template<class R, class ElementType>
struct is_span_compatible_range_impl<R, ElementType, typename std::enable_if<is_span_contiguous_range<R>::value && has_span_member_size<R>::value>::type>
    : bool_constant<
          (std::is_lvalue_reference<R>::value || std::is_const<ElementType>::value)
          && is_span_element_convertible<span_range_element_t<R>, ElementType>::value> {};

#endif

template<class R, class ElementType>
struct is_span_compatible_range : conjunction<negation<is_span_range_excluded<R>>, is_span_compatible_range_impl<R, ElementType>> {};

template<std::size_t Extent, std::size_t Offset, std::size_t Count>
struct span_subspan_extent
    : std::integral_constant<std::size_t, Count != dynamic_extent ? Count : (Extent != dynamic_extent ? Extent - Offset : dynamic_extent)> {};

[[noreturn]] inline void throw_span_out_of_range() { throw std::out_of_range("yk::polyfill::span::at: idx >= size()"); }

}  // namespace detail

template<class ElementType, std::size_t Extent>
class span : private detail::span_extent<Extent> {
  using extent_base = detail::span_extent<Extent>;

public:
  using element_type = ElementType;
  using value_type = typename std::remove_cv<ElementType>::type;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = element_type*;
  using const_pointer = element_type const*;
  using reference = element_type&;
  using const_reference = element_type const&;
  using iterator = detail::span_iterator<element_type>;
  using reverse_iterator = std::reverse_iterator<iterator>;
#if __cpp_lib_ranges_as_const >= 202207L
  using const_iterator = std::const_iterator<iterator>;
  using const_reverse_iterator = std::const_iterator<reverse_iterator>;
#else
  using const_iterator = detail::span_iterator<element_type const>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
#endif

  static constexpr size_type extent = Extent;

  // [span.cons]

  template<std::size_t E = Extent, typename std::enable_if<E == dynamic_extent || E == 0, std::nullptr_t>::type = nullptr>
  constexpr span() noexcept : extent_base(0), data_(nullptr)
  {
  }

  template<
      class It, std::size_t E = Extent,
      typename std::enable_if<E == dynamic_extent && detail::is_span_compatible_iterator<It, element_type>::value, std::nullptr_t>::type = nullptr>
  constexpr span(It first, size_type count) : extent_base(count), data_(detail::span_to_address(first))
  {
  }

  template<
      class It, std::size_t E = Extent,
      typename std::enable_if<E != dynamic_extent && detail::is_span_compatible_iterator<It, element_type>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit span(It first, size_type count) : extent_base(count), data_(detail::span_to_address(first))
  {
  }

  template<
      class It, class End, std::size_t E = Extent,
      typename std::enable_if<E == dynamic_extent && detail::is_span_compatible_sentinel<End, It, element_type>::value, std::nullptr_t>::type = nullptr>
  constexpr span(It first, End last) : extent_base(static_cast<size_type>(last - first)), data_(detail::span_to_address(first))
  {
  }

  template<
      class It, class End, std::size_t E = Extent,
      typename std::enable_if<E != dynamic_extent && detail::is_span_compatible_sentinel<End, It, element_type>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit span(It first, End last) : extent_base(static_cast<size_type>(last - first)), data_(detail::span_to_address(first))
  {
  }

  template<std::size_t N, typename std::enable_if<Extent == dynamic_extent || N == Extent, std::nullptr_t>::type = nullptr>
  constexpr span(typename type_identity<element_type>::type (&arr)[N]) noexcept : extent_base(N), data_(arr)
  {
  }

  // std::array::data() is constexpr since C++17.
  template<
      class T, std::size_t N,
      typename std::enable_if<(Extent == dynamic_extent || N == Extent) && detail::is_span_element_convertible<T, element_type>::value, std::nullptr_t>::type =
          nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR span(std::array<T, N>& arr) noexcept : extent_base(N), data_(arr.data())
  {
  }

  template<
      class T, std::size_t N,
      typename std::enable_if<
          (Extent == dynamic_extent || N == Extent) && detail::is_span_element_convertible<T const, element_type>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR span(std::array<T, N> const& arr) noexcept : extent_base(N), data_(arr.data())
  {
  }

  template<
      class R, std::size_t E = Extent,
      typename std::enable_if<E == dynamic_extent && detail::is_span_compatible_range<R, element_type>::value, std::nullptr_t>::type = nullptr>
  constexpr span(R&& r) : extent_base(detail::span_range_size(r)), data_(detail::span_range_data(r))
  {
  }

  template<
      class R, std::size_t E = Extent,
      typename std::enable_if<E != dynamic_extent && detail::is_span_compatible_range<R, element_type>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit span(R&& r) : extent_base(detail::span_range_size(r)), data_(detail::span_range_data(r))
  {
  }

  constexpr span(span const& other) noexcept = default;

  template<
      class OtherElementType, std::size_t OtherExtent,
      typename std::enable_if<
          (Extent == dynamic_extent || Extent == OtherExtent) && detail::is_span_element_convertible<OtherElementType, element_type>::value,
          std::nullptr_t>::type = nullptr>
  constexpr span(span<OtherElementType, OtherExtent> const& s) noexcept : extent_base(s.size()), data_(s.data())
  {
  }

  template<
      class OtherElementType, std::size_t OtherExtent,
      typename std::enable_if<
          Extent != dynamic_extent && OtherExtent == dynamic_extent && detail::is_span_element_convertible<OtherElementType, element_type>::value,
          std::nullptr_t>::type = nullptr>
  constexpr explicit span(span<OtherElementType, OtherExtent> const& s) noexcept : extent_base(s.size()), data_(s.data())
  {
  }

  YK_POLYFILL_CXX14_CONSTEXPR span& operator=(span const& other) noexcept = default;

  // [span.sub]

  template<std::size_t Count>
  constexpr span<element_type, Count> first() const
  {
    static_assert(Count <= Extent, "[span.sub] mandates Count <= Extent");
    return span<element_type, Count>(data(), Count);
  }

  template<std::size_t Count>
  constexpr span<element_type, Count> last() const
  {
    static_assert(Count <= Extent, "[span.sub] mandates Count <= Extent");
    return span<element_type, Count>(data() + (size() - Count), Count);
  }

  template<std::size_t Offset, std::size_t Count = dynamic_extent>
  constexpr span<element_type, detail::span_subspan_extent<Extent, Offset, Count>::value> subspan() const
  {
    static_assert(
        Offset <= Extent && (Count == dynamic_extent || Count <= Extent - Offset),
        "[span.sub] mandates Offset <= Extent && (Count == dynamic_extent || Count <= Extent - Offset)"
    );
    return span<element_type, detail::span_subspan_extent<Extent, Offset, Count>::value>(data() + Offset, Count != dynamic_extent ? Count : size() - Offset);
  }

  constexpr span<element_type, dynamic_extent> first(size_type count) const { return span<element_type, dynamic_extent>(data(), count); }

  constexpr span<element_type, dynamic_extent> last(size_type count) const { return span<element_type, dynamic_extent>(data() + (size() - count), count); }

  constexpr span<element_type, dynamic_extent> subspan(size_type offset, size_type count = dynamic_extent) const
  {
    return span<element_type, dynamic_extent>(data() + offset, count == dynamic_extent ? size() - offset : count);
  }

  // [span.obs]

  constexpr size_type size() const noexcept { return extent_base::stored_size(); }

  constexpr size_type size_bytes() const noexcept { return size() * sizeof(element_type); }

  YK_POLYFILL_NODISCARD constexpr bool empty() const noexcept { return size() == 0; }

  // [span.elem]

  constexpr reference operator[](size_type idx) const { return data_[idx]; }

  // Not `cond ? data_[idx] : throw ...`: without /Zc:ternary (off before /std:c++20), MSVC appears to yield a temporary copy
  // there, so the returned reference would dangle.
  constexpr reference at(size_type idx) const { return idx < size() ? data_[idx] : (detail::throw_span_out_of_range(), data_[idx]); }

  constexpr reference front() const { return *data_; }

  constexpr reference back() const { return data_[size() - 1]; }

  constexpr pointer data() const noexcept { return data_; }

  // [span.iterators]

  constexpr iterator begin() const noexcept { return iterator(data_); }

  constexpr iterator end() const noexcept { return iterator(data_ + size()); }

  constexpr const_iterator cbegin() const noexcept { return begin(); }

  constexpr const_iterator cend() const noexcept { return end(); }

  // std::reverse_iterator is constexpr since C++17.
  YK_POLYFILL_CXX17_CONSTEXPR reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }

  YK_POLYFILL_CXX17_CONSTEXPR reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

  YK_POLYFILL_CXX17_CONSTEXPR const_reverse_iterator crbegin() const noexcept { return rbegin(); }

  YK_POLYFILL_CXX17_CONSTEXPR const_reverse_iterator crend() const noexcept { return rend(); }

private:
  pointer data_;
};

#if !(__cpp_inline_variables >= 201606L)
template<class ElementType, std::size_t Extent>
constexpr std::size_t span<ElementType, Extent>::extent;
#endif

#if __cpp_deduction_guides >= 201703L

namespace detail {

// integral-constant-like ([span.syn])
template<class T, class = void>
struct is_integral_constant_like : false_type {};

template<class T>
struct is_integral_constant_like<T, void_t<decltype(T::value), bool_constant<T() == T::value>, bool_constant<static_cast<decltype(T::value)>(T()) == T::value>>>
    : bool_constant<
          std::is_integral<typename remove_cvref<decltype(T::value)>::type>::value
          && !std::is_same<bool, typename remove_cvref<decltype(T::value)>::type>::value && std::is_convertible<T, decltype(T::value)>::value> {};

// maybe-static-ext ([span.syn])
template<class T, class = void>
struct span_maybe_static_ext : std::integral_constant<std::size_t, dynamic_extent> {};

template<class T>
struct span_maybe_static_ext<T, typename std::enable_if<is_integral_constant_like<T>::value>::type>
    : std::integral_constant<std::size_t, std::size_t{T::value}> {};

}  // namespace detail

template<class It, class EndOrSize, typename std::enable_if<detail::is_span_contiguous_iterator<It>::value, std::nullptr_t>::type = nullptr>
span(It, EndOrSize) -> span<typename std::remove_reference<detail::span_iter_reference_t<It>>::type, detail::span_maybe_static_ext<EndOrSize>::value>;

template<class T, std::size_t N>
span(T (&)[N]) -> span<T, N>;

template<class T, std::size_t N>
span(std::array<T, N>&) -> span<T, N>;

template<class T, std::size_t N>
span(std::array<T, N> const&) -> span<T const, N>;

template<class R, typename std::enable_if<detail::is_span_contiguous_range<R>::value, std::nullptr_t>::type = nullptr>
span(R&&) -> span<detail::span_range_element_t<R>>;

#endif

#if __cpp_lib_byte >= 201603L

namespace detail {

template<class ElementType, std::size_t Extent>
struct span_bytes_extent : std::integral_constant<std::size_t, Extent == dynamic_extent ? dynamic_extent : sizeof(ElementType) * Extent> {};

}  // namespace detail

template<class ElementType, std::size_t Extent, typename std::enable_if<!std::is_volatile<ElementType>::value, std::nullptr_t>::type = nullptr>
span<std::byte const, detail::span_bytes_extent<ElementType, Extent>::value> as_bytes(span<ElementType, Extent> s) noexcept
{
  return span<std::byte const, detail::span_bytes_extent<ElementType, Extent>::value>{reinterpret_cast<std::byte const*>(s.data()), s.size_bytes()};
}

template<
    class ElementType, std::size_t Extent,
    typename std::enable_if<!std::is_const<ElementType>::value && !std::is_volatile<ElementType>::value, std::nullptr_t>::type = nullptr>
span<std::byte, detail::span_bytes_extent<ElementType, Extent>::value> as_writable_bytes(span<ElementType, Extent> s) noexcept
{
  return span<std::byte, detail::span_bytes_extent<ElementType, Extent>::value>{reinterpret_cast<std::byte*>(s.data()), s.size_bytes()};
}

#endif

}  // namespace polyfill

}  // namespace yk

#ifdef YK_POLYFILL_DETAIL_SPAN_USE_RANGES

namespace std {

namespace ranges {

template<class ElementType, std::size_t Extent>
inline constexpr bool enable_view<yk::polyfill::span<ElementType, Extent>> = true;

template<class ElementType, std::size_t Extent>
inline constexpr bool enable_borrowed_range<yk::polyfill::span<ElementType, Extent>> = true;

}  // namespace ranges

}  // namespace std

#  undef YK_POLYFILL_DETAIL_SPAN_USE_RANGES
#endif

#endif  // YK_ZZ_POLYFILL_SPAN_HPP
