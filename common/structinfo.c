#include <string.h>

#include "structinfo.h"


static inline int StructInfo_match_ (
    const struct StructInfo *info, const char *key) {
  return strcmp(info->key, key);
}


int StructInfo_match (const void *info, const void *key) {
  return StructInfo_match_(info, key);
}
