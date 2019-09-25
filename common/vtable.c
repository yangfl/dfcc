#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __GLIBC__
#include <execinfo.h>
#endif

#include "vtable.h"


VTable_hash_t VTable_rand = 0;


extern inline VTable_hash_t VTable_checksum (const struct VTable *this, size_t size);
extern inline bool VTable_vaild (const struct VTable *this, size_t size);
extern inline int VTable_init (struct VTable *this, size_t size);


VTable_hash_t VTable_hash (const void *buf, size_t size) {
  VTable_hash_t hash = 0;
  size_t number_of_int = size / sizeof(VTable_hash_t);
  size_t pad = size % sizeof(VTable_hash_t);
  for (size_t i = 0; i < number_of_int; i++) {
    hash ^= ((VTable_hash_t *) buf)[i];
  }
  if (pad != 0) {
    hash ^= ((VTable_hash_t *) buf)[number_of_int] & ~htole32(-1 << pad);
  }
  return hash;
}


void VTable_rand_init (void) {
  if unlikely (VTable_rand == 0) {
    srand(time(0));
    do {
      VTable_rand = rand();
    } while (VTable_rand == 0);
  }
}


static inline void print_trace (void) {
#ifdef __GLIBC__
  void *bt[32];
  backtrace_symbols_fd(bt + 3, backtrace(bt, sizeof(bt)) - 3, fileno(stderr));
#else
  fputs("backtrace unavailable on non-glibc", stderr);
#endif
}


int VTable_crash (void) {
  fputs("VTableCrash!", stderr);
  print_trace();
  exit(255);
}
