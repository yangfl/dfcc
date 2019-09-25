#ifndef CLASS_H
#define CLASS_H

#include "malloc.h"
#include "_class.h"


#define PROTECT_RETURN(this) if unlikely ((this) == NULL) return
#define __GENERATE_NEW_FUNC(Class, Struct, variant, args, ...) \
  static inline Struct *CONCAT(CONCAT(Class, _new), variant) args { \
    Struct *ret = malloc_et(Struct); \
    if likely (ret) { \
      CONCAT(CONCAT(Class, _init), variant)(ret, ## __VA_ARGS__); \
    } \
    return ret; \
  }
#define GENERATE_NEW_FUNC_GENERIC(Class, Struct, variant, args, ...) \
  __GENERATE_NEW_FUNC(Class, Struct      , CONCAT(_, variant), args, __VA_ARGS__)
#define GENERATE_NEW_FUNC(Class) \
  __GENERATE_NEW_FUNC(Class, struct Class,                   , ())
#define GENERATE_FREE_FUNC(Class) \
  static inline void Class ## _free (void *this) { \
    Class ## _destory(this); \
    free(this); \
  }


#endif /* CLASS_H */
