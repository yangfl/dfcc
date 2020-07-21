#ifndef TYPEINFO_H
#define TYPEINFO_H
/**
 * @ingroup CommonMacro
 * @defgroup StructInfo Struct Info
 * @brief Runtime struct info.
 * @{
 */

#include <stdbool.h>

#include <glib-object.h>


struct StructInfo {
  char *key;
  GType type;
  int offset;
};


#define G_TYPE_FROM(x) _Generic((x), \
  char: G_TYPE_CHAR, \
  unsigned char: G_TYPE_UCHAR, \
  bool: G_TYPE_BOOLEAN, \
  int: G_TYPE_INT, \
  unsigned int: G_TYPE_UINT, \
  long: G_TYPE_LONG, \
  unsigned long: G_TYPE_ULONG, \
  long long: G_TYPE_INT64, \
  unsigned long long: G_TYPE_UINT64, \
  float: G_TYPE_FLOAT, \
  double: G_TYPE_DOUBLE, \
  char *: G_TYPE_STRING, \
  char **: G_TYPE_STRV, \
  GVariant *: G_TYPE_VARIANT, \
  void *: G_TYPE_POINTER, \
  default: G_TYPE_INVALID \
)
#define STRUCT_INFO_STRING .type = G_TYPE_STRING

#define STRUCT_INFO_ADD(type, member, typeinfo) \
  {.key = # member, .offset = offsetof(type, member), typeinfo}
#define STRUCT_INFO(member) STRUCT_INFO_ADD(STRUCT_INFO_TYPE, member, .type = G_TYPE_FROM(((STRUCT_INFO_TYPE *) 0)->member))
#define STRUCT_INFO_OFTYPE(member, typeinfo) STRUCT_INFO_ADD(STRUCT_INFO_TYPE, member, typeinfo)

#define STRUCT_INFO_END {0}


//! @memberof StructInfo
int StructInfo_match (const void *info, const void *key);


/**@}*/

#endif /* TYPEINFO_H */

