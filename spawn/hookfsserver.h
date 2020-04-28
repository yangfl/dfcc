#ifndef DFCC_SPAWN_HOOKFS_SERVER_H
#define DFCC_SPAWN_HOOKFS_SERVER_H

#include <gio/gio.h>


#define HOOKFS_PULL (0u)
#define HOOKFS_PUSH (1u)

#define HOOKFS_PATH (0u)
#define HOOKFS_HASH (1u << 1)


typedef const char *(*HookFsServer__FileTranslator) (
  unsigned long long id, const char *path, int mode);


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
