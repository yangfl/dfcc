#include <stddef.h>

#include "common/macro.h"
#include "prepost.h"


struct StructInfo Result__info[] = {
  {"source_hash", G_TYPE_UINT64, offsetof(struct Result, source_hash)},
  {"source_normalized_hash", G_TYPE_UINT64, offsetof(struct Result, source_normalized_hash)},
  {"preprocessed_hash", G_TYPE_UINT64, offsetof(struct Result, preprocessed_hash)},
  {"preprocessed_normalized_hash", G_TYPE_UINT64, offsetof(struct Result, preprocessed_normalized_hash)},
  {"object_hash", G_TYPE_UINT64, offsetof(struct Result, object_hash)},
};
const int Result__info_n = G_N_ELEMENTS(Result__info);


int Client_pre (struct Config *config) {
  return 1;
}


void Client_post (struct Config *config, struct Result *result) {

}
