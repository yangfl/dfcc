#ifndef STRUCTINFO_H
#define STRUCTINFO_H
/**
 * @ingroup CommonMacro
 * @defgroup StructInfo Struct Info
 * @brief Runtime struct info.
 * @{
 */

#include <glib-object.h>


struct StructInfo {
  char *key;
  GType type;
  int offset;
};


//! @memberof StructInfo
int StructInfo_match (const void *info, const void *key);


/**@}*/

#endif /* STRUCTINFO_H */

