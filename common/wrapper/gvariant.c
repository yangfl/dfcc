#include <stdbool.h>

#include <glib.h>

#include "common/macro.h"
#include "common/typeinfo.h"
#include "./version.h"
#include "gvariant.h"


GVariant *g_variant_new_struct (const void *instance, struct StructInfo *info) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

#define case_type(TYPE, type, func) \
  case G_TYPE_ ## TYPE: { \
    type value = G_STRUCT_MEMBER(type, instance, info[i].offset); \
    if (value) { \
      g_variant_builder_add( \
        &builder, "{sv}", \
        info[i].key, g_variant_new_ ## func(value)); \
    } \
    break; \
  }

  for (int i = 0; info[i].key != NULL; i++) {
    switch (info[i].type) {
      case_type(CHAR, char, byte)
      case_type(UCHAR, char, byte)
      case_type(BOOLEAN, bool, boolean)
      case_type(INT, int, int32)
      case_type(UINT, unsigned int, uint32)
      case_type(LONG, long, int32)
      case_type(ULONG, unsigned long, int32)
      case_type(INT64, long long, int64)
      case_type(UINT64, unsigned long long, uint64)
      case_type(FLOAT, float, double)
      case_type(DOUBLE, double, double)
      case_type(STRING, const char *, string)
      default:
        g_log(DFCC_NAME "-GVariant", G_LOG_LEVEL_WARNING,
              "Unknown type '%s' for key '%s'",
              g_type_name(info[i].type), info[i].key);
    }
  }

#undef case_type

  return g_variant_builder_end(&builder);
}


void *g_variant_get_struct (
    GVariant *value, void *instance, struct StructInfo *info) {
  return_if_g_variant_not_type(value, "a{sv}", DFCC_NAME "-GVariant") NULL;

#define case_type_value(TYPE, type, value) \
  case G_TYPE_ ## TYPE: { \
    type value_ = value; \
    if (value_) { \
      G_STRUCT_MEMBER(type, instance, info->offset) = value_; \
    } \
    break; \
  }

#define case_type(TYPE, type, func) \
  case_type_value(TYPE, type, g_variant_get_ ## func(value))

  GVariantDict dict;
  g_variant_dict_init(&dict, value);

  for (int i = 0; info[i].key != NULL; i++) {
    GVariant *v = g_variant_dict_lookup_value(
      &dict, info[i].key, G_VARIANT_TYPE_VARIANT);
    if (v == NULL) {
      continue;
    }
    switch (info[i].type) {
      case_type(CHAR, char, byte)
      case_type(UCHAR, char, byte)
      case_type(BOOLEAN, bool, boolean)
      case_type(INT, int, int32)
      case_type(UINT, unsigned int, uint32)
      case_type(LONG, long, int32)
      case_type(ULONG, unsigned long, int32)
      case_type(INT64, long long, int64)
      case_type(UINT64, unsigned long long, uint64)
      case_type(FLOAT, float, double)
      case_type(DOUBLE, double, double)
      case_type_value(STRING, char *, g_variant_dup_string(value, NULL))
      default:
        g_log(DFCC_NAME "-GVariant", G_LOG_LEVEL_WARNING,
              "Unknown type '%s' for key '%s'",
              g_type_name(info[i].type), info[i].key);
    }
  }

#undef case_type

  g_variant_dict_clear(&dict);
  return instance;
}
