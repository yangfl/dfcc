#ifndef MACRO_H
#define MACRO_H
/**
 * @defgroup Common Common
 * @brief Helper functions, data structures and macros.
 * @{
 * @defgroup CommonMacro Common Macro
 * @brief Useful macro definitions.
 * @{
 */


/**
 * @brief Hints the compiler that the expression is likely to evaluate to a true
 *        value.
 *
 * The compiler may use this information for optimizations.
 *
 * @code{.c}
  if likely (random() != 1)
    puts("not one");
 * @endcode
 *
 * @param x the expression
 * @return the value of `x`
 */
#define likely(x)       (__builtin_expect(!!(x), 1))
/**
 * @brief Hints the compiler that the expression is unlikely to evaluate to a
 *        true value.
 *
 * The compiler may use this information for optimizations.
 *
 * @code{.c}
  if unlikely (random() == 1)
    puts("a random one");
 * @endcode
 *
 * @param x the expression
 * @return the value of `x`
 */
#define unlikely(x)     (__builtin_expect(!!(x), 0))

/**
 * @brief Indicate a test should success.
 *
 * If the test fails, an error should have happened.
 *
 * This keyword should always be used with `otherwise`.
 *
 * @code{.c}
  should (fopen("/tmp/a", "w") != NULL) otherwise puts("Cannot open file!");
 * @endcode
 *
 * @param test the test expression
 */
#define should(test) if likely (test)
/**
 * @brief Tell what to do if an error happens.
 *
 * This keyword should always be used with `should`.
 *
 * @code{.c}
  should (fopen("/tmp/a", "w") != NULL) otherwise puts("Cannot open file!");
 * @endcode
 */
#define otherwise ; else

/**
 * @brief Make a breakable code block.
 *
 * `break` statement is useable in the block.
 *
 * @code{.c}
  if (write_log) do_once {
    if (log_does_not_exist) break;
    write();
  }
 * @endcode
 */
#define do_once switch (1) case 1:

/**
 * @brief Calculates the maximum of `a` and `b`.
 *
 * @param a a numeric value
 * @param b a numeric value
 * @return the maximum
 */
#define max(a,b) \
  ({ __auto_type _a = (a); \
      __auto_type _b = (b); \
    _a > _b ? _a : _b; })
/**
 * @brief Calculates the minimum of `a` and `b`.
 *
 * @param a a numeric value
 * @param b a numeric value
 * @return the minimum
 */
#define min(a,b) \
  ({ __auto_type _a = (a); \
      __auto_type _b = (b); \
    _a < _b ? _a : _b; })

/**
 * @}
 * @}
 */
#endif /* MACRO_H */
