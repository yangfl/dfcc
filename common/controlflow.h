#ifndef CONTROLFLOW_H
#define CONTROLFLOW_H
/**
 * @ingroup CommonMacro
 * @defgroup ControlFlow Control Flow
 * @brief Meaningful control flow statements.
 * @{
 */

#define not(expr) (!(expr))

#define return_if(expr) if (expr) return
#define return_if_not(expr) if (!(expr)) return
#define return_if_fail(expr) if unlikely (!(expr)) return

#define break_if(expr) if (expr) break
#define break_if_not(expr) if (!(expr)) break
#define break_if_fail(expr) if unlikely (!(expr)) break

#define continue_if(expr) if (expr) continue
#define continue_if_not(expr) if (!(expr)) continue
#define continue_if_fail(expr) if unlikely (!(expr)) continue

#define goto_if(expr) if (expr) goto
#define goto_if_not(expr) if (!(expr)) goto
#define goto_if_fail(expr) if unlikely (!(expr)) goto

#define unless(expr) while (!(expr))

/**@}*/

#endif /* CONTROLFLOW_H */
