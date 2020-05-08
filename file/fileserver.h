#ifndef DFCC_FILE_FILESERVER_H
#define DFCC_FILE_FILESERVER_H

#include <gmodule.h>

//#include "server/session.h"
#include "file/entry.h"


//! @ingroup File
struct FileServer {
  GHashTable *table;
  struct Cache *cache;

  void (*onmissing) (struct HookedSubprocess *, void *, char *);
  void *onmissing_userdata;
};

int FileServer_get (struct FileServer *fileserver, guint ns, const char *path) {
  g_hash_table_lookup(fileserver->table, &ns);
}


#endif /* DFCC_FILE_FILESERVER_H */

