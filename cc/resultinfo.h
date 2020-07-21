#ifndef DFCC_CC_RESULT_INFO_H
#define DFCC_CC_RESULT_INFO_H

#include "common/typeinfo.h"
#include "file/hash.h"


/// @ingroup CC
struct ResultInfo {
  FileHash source_hash;
  FileHash source_normalized_hash;
  FileHash preprocessed_hash;
  FileHash preprocessed_normalized_hash;
  FileHash object_hash;
};


extern struct StructInfo ResultInfo__info[];


#endif /* DFCC_CC_RESULT_INFO_H */
