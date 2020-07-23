#ifndef CDECLS_H
#define CDECLS_H

/**
 * @ingroup CommonMacro
 * @defgroup CDecls Warp C functions
 * @brief BEGIN_C_DECLS and END_C_DECLS
 * @{
 */


#ifdef __cplusplus
/**
 * @brief Used (along with C_END_DECLS) to bracket header files.
 *
 * If the compiler in use is a C++ compiler, adds extern "C" around the header.
 */
# define BEGIN_C_DECLS extern "C" {
/**
 * @brief Used (along with C_BEGIN_DECLS) to bracket header files.
 *
 * If the compiler in use is a C++ compiler, adds extern "C" around the header.
 */
# define END_C_DECLS }
# define C_INLINE(decl, body) decl;
# define C_ONLY(...) /* empty */
# define CXX_ONLY(x) x
# define ANON_MEMBER __anon
#else
# define BEGIN_C_DECLS /* empty */
# define END_C_DECLS /* empty */
# define C_INLINE(decl, body) inline decl body
# define C_ONLY(x) x
# define CXX_ONLY(...) /* empty */
# define ANON_MEMBER /* empty */
#endif


/**@}*/

#endif /* CDECLS_H */

