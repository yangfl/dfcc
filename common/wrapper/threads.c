#include "threads.h"


extern inline int mtx_init_e (mtx_t *mutex, int type, GError **error);
extern inline int mtx_lock_e (mtx_t *mutex, GError **error);
extern inline int mtx_unlock_e (mtx_t *mutex, GError **error);

extern inline int cnd_init_e (cnd_t* cond, GError **error);
extern inline int cnd_signal_e (cnd_t* cond, GError **error);
extern inline int cnd_broadcast_e (cnd_t* cond, GError **error);
extern inline int cnd_wait_e (cnd_t* cond, mtx_t* mutex, GError **error);
