#include <cstring>
#include <gtest/gtest.h>

#include "common/hexstring.h"


TEST(Common, hexstring) {
  EXPECT_EQ(chr2hex(0), '0');
  EXPECT_EQ(chr2hex(0xf), 'f');

  const char src[] = {0x11, 0x22, 0x33, 0x45};
  char dst[sizeof(src) * 2 + 1] = {0};
  buf2hex(dst, (void *) src, sizeof(src));
  EXPECT_STREQ(dst, "11223345");
}
