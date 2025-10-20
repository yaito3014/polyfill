#ifndef YK_POLYFILL_CONFIG_HPP
#define YK_POLYFILL_CONFIG_HPP

#if __cplusplus >= 201402L
#define YK_POLYFILL_CXX14_CONSTEXPR constexpr
#else
#define YK_POLYFILL_CXX14_CONSTEXPR
#endif

#if __cplusplus >= 202002L
#define YK_POLYFILL_CXX20_CONSTEXPR constexpr
#else
#define YK_POLYFILL_CXX20_CONSTEXPR
#endif

#endif  // YK_POLYFILL_CONFIG_HPP
