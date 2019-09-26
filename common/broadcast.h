#ifndef DFCC_COMMON_BROADCAST_H
#define DFCC_COMMON_BROADCAST_H

#include <gmodule.h>


typedef void *(*QueryFunc)(void *, const void *);
typedef void *(*DupFunc)(const void *);


struct Broadcast {
  GHashTable *channels;
  void *(*key_dup_func) (const void *);
  GMutex mutex;
};


void *Broadcast_listen (
    struct Broadcast *sta, const void *channel,
    QueryFunc query, void *query_data);
void Broadcast_send (struct Broadcast *sta, const void *channel, void *message);
void Broadcast_destroy (struct Broadcast *sta);
int Broadcast_init (
    struct Broadcast *sta, GHashFunc hash_func, GEqualFunc key_equal_func,
    DupFunc key_dup_func, GDestroyNotify key_destroy_func);


#endif /* DFCC_COMMON_BROADCAST_H */

