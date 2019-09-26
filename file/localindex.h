#ifndef DFCC_FILE_LOCAL_INDEX_H
#define DFCC_FILE_LOCAL_INDEX_H

#include <gmodule.h>

#include "entry.h"


struct FileEntryE *LocalFileIndex_get (
    GHashTable *index, const char* path, GError **error);

#define LocalFileIndex_free g_hash_table_destroy
#define LocalFileIndex_new() g_hash_table_new_full(g_str_hash, g_str_equal, NULL, FileTag_free)

#endif /* DFCC_FILE_LOCAL_INDEX_H */
