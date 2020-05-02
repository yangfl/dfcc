#ifndef DFCC_SPAWN_HOOKFS_SERVER_H
#define DFCC_SPAWN_HOOKFS_SERVER_H

#include <gio/gio.h>


//! @memberof HookFsServer
#define HOOKFS_PULL (0u)
//! @memberof HookFsServer
#define HOOKFS_PUSH (1u)


//! @memberof HookFsServer
typedef unsigned long long HookFsID;
//! @memberof HookFsServer
typedef const char *(*HookFsServer__FileTranslator) (
  HookFsID id, const char *path, int mode);


//! @ingroup Spawn
struct HookFsServer {
  GSocketService *service;
  HookFsServer__FileTranslator translator;
  char *path;
};


//! @memberof HookFsServer
void HookFsServer_destroy (struct HookFsServer *server);
//! @memberof HookFsServer
int HookFsServer_init (
    struct HookFsServer *server, const char *path,
    HookFsServer__FileTranslator translator, GError **error);


#endif /* DFCC_SPAWN_HOOKFS_SERVER_H */
