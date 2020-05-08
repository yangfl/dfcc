#include <libsoup/soup.h>

#include "common/macro.h"
#include "common/wrapper/soup.h"
#include "file/entry.h"
#include "file/remoteindex.h"
#include "../../protocol.h"
#include "../../log.h"
#include "associate.h"


void Server_rpc_associate (
    struct ServerContext *server_ctx, struct Session *session,
    SoupMessage *msg, GVariant *param) {
  GVariantIter iter;
  gchar *path;
  size_t size;
  FileHash hash;
  for (g_variant_iter_init(&iter, param);
       g_variant_iter_next(&iter, "{s(tt)}", &path, &size, &hash);) {
    if unlikely (!g_path_is_absolute(path)) {
      //warn
      g_free(path);
      continue;
    }

    struct FileTag *tag = g_new(struct FileTag, 1);
    FileTag_init_with_hash(tag, path, hash);
    should (RemoteFileIndex_add(
        &((struct HookedProcessGroup *) session)->file_index, tag, false)
    ) otherwise {
      FileTag_destroy(tag);
      g_free(tag);
      g_free(path);
    }
  }

  g_variant_unref(param);
}
