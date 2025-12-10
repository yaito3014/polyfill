#ifndef YK_POLYFILL_CONFIG_HPP
#define YK_POLYFILL_CONFIG_HPP

#if __cplusplus >= 201402L
#define YK_POLYFILL_CXX14_CONSTEXPR constexpr
#else
#define YK_POLYFILL_CXX14_CONSTEXPR
#endif

#if __cplusplus >= 201703L
#define YK_POLYFILL_CXX17_CONSTEXPR constexpr
#else
#define YK_POLYFILL_CXX17_CONSTEXPR
#endif

#if __cplusplus >= 202002L
#define YK_POLYFILL_CXX20_CONSTEXPR constexpr
#else
#define YK_POLYFILL_CXX20_CONSTEXPR
#endif

#if __cplusplus >= 202302L
#define YK_POLYFILL_CXX23_CONSTEXPR constexpr
#else
#define YK_POLYFILL_CXX23_CONSTEXPR
#endif

#if __cpp_inline_variables >= 201606L
#define YK_POLYFILL_INLINE inline
#else
#define YK_POLYFILL_INLINE static
#endif

#endif  // YK_POLYFILL_CONFIG_HPP
