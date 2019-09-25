#ifndef MACRO_H
#define MACRO_H

#define CONCAT_IMPL(x, y) x ## y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define GET_1ST_ARG(arg1, ...) arg1
#define GET_2ND_ARG(arg1, arg2, ...) arg2
#define GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
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
#define COMPARE(a, b) COMPARE_IMPL(a, b)

#define COMPARE_(x) x
#define COMPARE_x(x) x

#ifdef __GNUC__
#define unused __attribute__((unused))
#else
#define unused
#endif

#define auto __auto_type

#define likely(x)       (__builtin_expect(!!(x), 1))
#define unlikely(x)     (__builtin_expect(!!(x), 0))

#define max(a,b) \
  ({ __auto_type _a = (a); \
      __auto_type _b = (b); \
    _a > _b ? _a : _b; })
#define min(a,b) \
  ({ __auto_type _a = (a); \
      __auto_type _b = (b); \
    _a < _b ? _a : _b; })

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif /* MACRO_H */
