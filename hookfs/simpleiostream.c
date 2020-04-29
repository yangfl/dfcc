#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <macro.h>
#include <simplestring.h>

#include "simplestringio.h"


static struct SimpleString *SimpleString_prepare (struct SimpleString *s, size_t len) {
  if (s->allocated_len > len) {
    return s;
  }
  char *new_str = realloc(s->str, len + 1);
  should (new_str != NULL) otherwise {
    return NULL;
  }
  s->str = new_str;
  s->allocated_len = len + 1;
  return s;
}


static struct SimpleString *SimpleString_append_len (
    struct SimpleString * restrict s, const char * restrict str, size_t len) {
  should (SimpleString_prepare(s, s->len + len) != NULL) otherwise {
    return NULL;
  }
  memcpy(s->str + s->len, str, len);
  s->len += len;
  s->str[s->len] = '\0';
  return s;
}


static struct SimpleString *SimpleString_append (
    struct SimpleString * restrict s, const char * restrict str) {
  return SimpleString_append_len(s, str, strlen(str));
}


int sioprintf (
    struct SimpleString * restrict s, const char * restrict format, ...) {
  char *buf;
  va_list ap;
  va_start(ap, format);
  int ret = vasprintf(&buf, format, ap);
  va_end(ap);
  should (ret >= 0) otherwise {
    return ret;
  }

  SimpleString_append_len(s->str, *buf, ret);
  return ret;
}


size_t siowrite (
    const void * restrict ptr, size_t size, size_t count,
    struct SimpleString * restrict s) {
  size *= count;
  SimpleString_append_len(s->str, ptr, count);
  return size;
}


int sioscanf (
    struct SimpleString * restrict s, const char * restrict format, ...) {

}


size_t sioread (
    const void * restrict ptr, size_t size, size_t count,
    struct SimpleString * restrict s) {
  size *= count;
  if (s->i + size > s->len) {
    size = s->len - s->i;
  }
  memcpy(ptr, s->str + s->i, size);
  s->i += size;
  return size;
}
