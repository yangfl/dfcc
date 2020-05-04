#ifndef DFCC_CLIENT_PREPOST_H
#define DFCC_CLIENT_PREPOST_H
/**
 * @ingroup Client
 * @defgroup PrePost Pre/Post Actions
 * @{
 */

#include <structinfo.h>

#include "../config/config.h"
#include "../file/hash.h"


struct Result {
  FileHash source_hash;
  FileHash source_normalized_hash;
  FileHash preprocessed_hash;
  FileHash preprocessed_normalized_hash;
  FileHash object_hash;
};


extern struct StructInfo Result__info[];
extern const int Result__info_n;


int Client_pre (struct Config *config);
void Client_post (struct Config *config, struct Result *result);


/**@}*/

#endif /* DFCC_CLIENT_PREPOST_H */
