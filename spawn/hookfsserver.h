#ifndef DFCC_SPAWN_HOOKFS_SERVER_H
#define DFCC_SPAWN_HOOKFS_SERVER_H

#include <stdatomic.h>
#include <stdbool.h>

#include <gio/gio.h>

#include "spawn/_hookedprocessgroupid.h"

BEGIN_C_DECLS


//! @memberof HookFsServer
#define HOOKFS_READ (0u)
//! @memberof HookFsServer
#define HOOKFS_WRITE (1u)


//! @memberof HookFsServer
typedef char *(*HookFsServerFileTranslator) (
  struct HookedProcess *p, const char *path, bool dierction_read, int mode);
typedef struct HookedProcess *(*HookFsServerProcessResolver) (
  void *self, HookedProcessGroupID hgid, GPid pid);


//! @ingroup Spawn
struct HookFsServer {
  HookFsServerFileTranslator translator;
  HookFsServerProcessResolver resolver;
  GSocketService *service;
  char *socket_path;
  /// User data.
  void *userdata;
};


//! @memberof HookFsServer
void HookFsServer_destroy (struct HookFsServer *server);
//! @memberof HookFsServer
int HookFsServer_init (
  struct HookFsServer *server, const char *socket_path,
  HookFsServerFileTranslator translator, HookFsServerProcessResolver resolver,
  GError **error);


END_C_DECLS

#endif /* DFCC_SPAWN_HOOKFS_SERVER_H */
