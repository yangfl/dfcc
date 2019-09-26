#include <stdbool.h>

#include <gmodule.h>

#include "broadcast.h"


typedef struct _GMutexCond {
  GMutex mutex;
  GCond cond;
  void *message;
  unsigned int rc;
} GMutexCond;


void g_mutex_cond_clear (GMutexCond *mutexcond) {
  g_mutex_clear(&mutexcond->mutex);
  g_cond_clear(&mutexcond->cond);
}


void g_mutex_cond_init (GMutexCond *mutexcond) {
  g_mutex_init(&mutexcond->mutex);
  g_cond_init(&mutexcond->cond);
  mutexcond->message = NULL;
  mutexcond->rc = 0;
}


void *Broadcast_listen (
    struct Broadcast *sta, const void *channel,
    QueryFunc query, void *query_data) {
  void *message;

  g_mutex_lock(&sta->mutex);
  if (query != NULL) {
    message = query(query_data, channel);
    if (message != NULL) {
      g_mutex_unlock(&sta->mutex);
      return message;
    }
  }

  GMutexCond *mutexcond = g_hash_table_lookup(sta->channels, channel);
  if (mutexcond == NULL) {
    mutexcond = g_malloc(sizeof(GMutexCond));
    g_mutex_cond_init(mutexcond);
    g_hash_table_insert(sta->channels, sta->key_dup_func(channel), mutexcond);
  }
  g_mutex_unlock(&sta->mutex);

  g_mutex_lock(&mutexcond->mutex);
  mutexcond->rc++;
  do {
    g_cond_wait(&mutexcond->cond, &mutexcond->mutex);
  } while (mutexcond->message == NULL);
  message = mutexcond->message;
  mutexcond->rc--;
  bool clear = mutexcond->rc == 0;
  g_mutex_unlock(&mutexcond->mutex);

  if (clear) {
    g_mutex_cond_clear(mutexcond);
    g_free(mutexcond);
  }

  return message;
}


void Broadcast_send (struct Broadcast *sta, const void *channel, void *message) {
  g_mutex_lock(&sta->mutex);
  GMutexCond *mutexcond = g_hash_table_lookup(sta->channels, channel);
  if (mutexcond == NULL) {
    g_mutex_unlock(&sta->mutex);
    return;
  }
  g_hash_table_remove(sta->channels, channel);
  g_mutex_unlock(&sta->mutex);

  g_mutex_lock(&mutexcond->mutex);
  mutexcond->message = message;
  g_cond_broadcast(&mutexcond->cond);
  g_mutex_unlock(&mutexcond->mutex);
}


void Broadcast_destroy (struct Broadcast *sta) {
  g_hash_table_destroy(sta->channels);
  g_mutex_clear(&sta->mutex);
}


int Broadcast_init (
    struct Broadcast *sta, GHashFunc hash_func, GEqualFunc key_equal_func,
    DupFunc key_dup_func, GDestroyNotify key_destroy_func) {
  sta->channels =
    g_hash_table_new_full(hash_func, key_equal_func, key_destroy_func, NULL);
  g_mutex_init(&sta->mutex);
  sta->key_dup_func = key_dup_func;
  return 0;
}
