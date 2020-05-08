#ifndef DFCC_SPAWN_HOOKFS_SERVER_H
#define DFCC_SPAWN_HOOKFS_SERVER_H

#include <stdbool.h>

#include <gio/gio.h>


//! @memberof HookFsServer
#define HOOKFS_READ (0u)
//! @memberof HookFsServer
#define HOOKFS_WRITE (1u)


//! @memberof HookFsServer
typedef unsigned long long HookFsID;
//! @memberof HookFsServer
typedef char *(*HookFsServerFileTranslator) (
  HookFsID, const char *, bool, int);


//! @ingroup Spawn
struct HookFsServer {
  GSocketService *service;
  HookFsServerFileTranslator translator;
  char *path;
};


//! @memberof HookFsServer
void HookFsServer_destroy (struct HookFsServer *server);
//! @memberof HookFsServer
int HookFsServer_init (
    struct HookFsServer *server, const char *path,
    HookFsServerFileTranslator translator, GError **error);


#endif /* DFCC_SPAWN_HOOKFS_SERVER_H */
