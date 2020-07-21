#ifndef DFCC_WRAPPER_GVARIANT_H
#define DFCC_WRAPPER_GVARIANT_H

#include <glib.h>

#include "common/typeinfo.h"


#define return_if_g_variant_not_type(v, s, log_domain) \
  should (g_variant_is_of_type((v), G_VARIANT_TYPE(s))) otherwise \
    if (g_log((log_domain), G_LOG_LEVEL_WARNING, \
          "Expect type '%s', got '%s'", (s), g_variant_get_type_string(v)), 1) \
      return

GVariant *g_variant_new_struct (const void *instance, struct StructInfo *info);
void *g_variant_get_struct (
  GVariant *value, void *instance, struct StructInfo *info);


#endif /* DFCC_WRAPPER_GVARIANT_H */
