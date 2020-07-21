#include "common/typeinfo.h"
#include "resultinfo.h"


struct StructInfo ResultInfo__info[] = {
#define STRUCT_INFO_TYPE struct ResultInfo
  STRUCT_INFO(source_hash),
  STRUCT_INFO(source_normalized_hash),
  STRUCT_INFO(preprocessed_hash),
  STRUCT_INFO(preprocessed_normalized_hash),
  STRUCT_INFO(object_hash),
  STRUCT_INFO_END
#undef STRUCT_INFO_TYPE
};
