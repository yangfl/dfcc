#ifndef HACK_H
#define HACK_H
/**
 * @ingroup CommonMacro
 * @defgroup Hack Macro Hack
 * @brief Crazy macro definitions.
 */

#define CONCAT_IMPL(x, y) x ## y
/**
 * @ingroup Hack
 * @brief Safely concat two symbols `x` and `y`.
 *
 * @param x a symbol
 * @param y a symbol
 * @return concated symbol
 */
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define VA_NARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
/**
 * @ingroup Hack
 * @brief Return the number of arguements.
 *
 * @code{.c}
  VA_NARGS(arg1, arg2, arg3, arg4)  // -> 4
  VA_NARGS()  // -> 0
  VA_NARGS(__VA_ARGS__)
 * @endcode
 *
 * @return the number of arguements
 */
#define VA_NARGS(...) VA_NARGS_IMPL(x, ## __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/**
 * @ingroup Hack
 * @brief Get the first arguement.
 *
 * @return the first arguement
 */
#define GET_1ST_ARG(arg1, ...) arg1
/**
 * @ingroup Hack
 * @brief Get the second arguement.
 *
 * @return the second arguement
 */
#define GET_2ND_ARG(arg1, arg2, ...) arg2
/**
 * @ingroup Hack
 * @brief Get the third arguement.
 *
 * @return the third arguement
 */
#define GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
/**
 * @ingroup Hack
 * @brief Get the forth arguement.
 *
 * @return the forth arguement
 */
#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
/**
 * @ingroup Hack
 * @brief Get the fifth arguement.
 *
 * @return the fifth arguement
 */
#define GET_5TH_ARG(arg1, arg2, arg3, arg4, arg5, ...) arg5
#define GET_6TH_ARG(arg1, arg2, arg3, arg4, arg5, arg6, ...) arg6
#define GET_7TH_ARG(arg1, arg2, arg3, arg4, arg5, arg6, arg7, ...) arg7

#define IIF(c) CONCAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t

#define COMPL(b) CONCAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define BITAND(x) CONCAT(BITAND_, x)
#define BITAND_0(y) 0
#define BITAND_1(y) y

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,

#define IS_PAREN_PROBE(...) PROBE(~)
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)

#define COMPARE_IMPL(a, b) \
    IIF( \
        BITAND \
            (IS_PAREN(COMPARE_ ## a(()))) \
            (IS_PAREN(COMPARE_ ## b(()))) \
    )( \
        COMPL(IS_PAREN( \
            COMPARE_ ## a( \
                COMPARE_ ## b \
            )(()) \
        )), \
        0 \
    )
/**
 * @ingroup Hack
 * @brief Compare two symbols `a` and `b`.
 *
 * @param a a symbol
 * @param b a symbol
 * @return 1 if `a` and `b` are the same symbol
 */
#define COMPARE(a, b) COMPARE_IMPL(a, b)

#define COMPARE_(x) x
#define COMPARE_x(x) x

#define DUP_RETURN(ret) \
  _Pragma("GCC diagnostic push"); \
  _Pragma("GCC diagnostic ignored \"-Wint-conversion\""); \
  _Pragma("GCC diagnostic ignored \"-Wincompatible-pointer-types\""); \
  fprintf( \
    middle_outer, \
    _Generic((ret), \
      int: "%d\n", \
      void*: "%p\n", \
      FILE*: "%d\n", \
      DIR*: "%p\n" \
    ), \
    _Generic((ret), \
      FILE*: ((ret) ? fileno(ret) : 0), \
      default: (ret) \
    )); \
  _Pragma("GCC diagnostic pop"); \
  return (ret)


#endif /* HACK_H */
