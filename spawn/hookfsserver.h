#ifndef DFCC_SPAWN_HOOKFS_SERVER_H
#define DFCC_SPAWN_HOOKFS_SERVER_H

#include <gio/gio.h>


#define HOOKFS_PULL (0u)
#define HOOKFS_PUSH (1u)


typedef unsigned long long HookFsID;
typedef const char *(*HookFsServer__FileTranslator) (
  HookFsID id, const char *path, int mode);


struct HookFsServer {
  GSocketService *service;
  HookFsServer__FileTranslator translator;
  char *path;
};


void HookFsServer_destroy (struct HookFsServer *server);
int HookFsServer_init (
    struct HookFsServer *server, const char *path,
    HookFsServer__FileTranslator translator, GError **error);


#endif /* DFCC_SPAWN_HOOKFS_SERVER_H */
