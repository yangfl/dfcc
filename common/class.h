#ifndef CLASS_H
#define CLASS_H

#include <stdlib.h>

#include "class/_class.h"


#define PROTECT_RETURN(instance) if unlikely ((instance) == NULL) return

#define __GENERATE_NEW_FUNC(Class, Struct, variant, args, ...) \
  static inline Struct *CONCAT(CONCAT(Class, _new), variant) args { \
    Struct *instance = (Struct *) g_malloc(sizeof(Struct)); \
    CONCAT(CONCAT(Class, _init), variant)(instance, ## __VA_ARGS__); \
    return instance; \
  }
#define GENERATE_NEW_FUNC_GENERIC(Class, Struct, variant, args, ...) \
  __GENERATE_NEW_FUNC(Class, Struct      , CONCAT(_, variant), args, __VA_ARGS__)
#define GENERATE_NEW_FUNC(Class, args, ...) \
  __GENERATE_NEW_FUNC(Class, struct Class,                   , args, __VA_ARGS__)

#define GENERATE_FREE_FUNC_CUSTOM(Class, free) \
  static inline void Class ## _free (void *instance) { \
    Class ## _destroy(instance); \
    free(instance); \
  }
#define GENERATE_FREE_FUNC_DEFAULT(Class) GENERATE_FREE_FUNC_CUSTOM(Class, free)
#define GENERATE_FREE_FUNC_GENERIC(...) GET_3RD_ARG(__VA_ARGS__, GENERATE_FREE_FUNC_CUSTOM, GENERATE_FREE_FUNC_DEFAULT, )
#define GENERATE_FREE_FUNC(...) GENERATE_FREE_FUNC_GENERIC(__VA_ARGS__)(__VA_ARGS__)


#endif /* CLASS_H */
