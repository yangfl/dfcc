#ifndef _CLASS_H
#define _CLASS_H

#include "macro.h"
#include "vtable.h"


/*
CLASS(A, {
  INHERIT(B);
}) a;
CAST(B, a);
*/
#define CLASS(Class, ...) \
  typedef struct VTABLE_CLASSNAME(Class) VTABLE_CLASSNAME(Class); \
  typedef struct Class __VA_ARGS__ Class; \
  struct Class
#define SUPCLASS_FIELD(SupClass) __sup_ ## SupClass
#define INHERIT(SupClass) \
  union { \
    struct SupClass; \
    struct SupClass SUPCLASS_FIELD(SupClass); \
  }
#define CAST(SupClass, instance) (instance).SUPCLASS_FIELD(SupClass)


#endif /* _CLASS_H */
