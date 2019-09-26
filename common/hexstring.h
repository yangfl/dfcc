#ifndef HEXSTRING_H
#define HEXSTRING_H


inline char chr2hex (unsigned char c) {
  if (c >= 0xA) {
    return c + 'a' - 0xA;
  } else {
    return c + '0';
  }
}


inline void buf2hex (char *dst, const void *src, unsigned int len) {
  const unsigned char *src_ = src;
  for (unsigned int i = 0; i < len; i++) {
    dst[2 * i] = chr2hex(src_[i] >> 4);
    dst[2 * i + 1] = chr2hex(src_[i] & 0xF);
  }
}


#endif /* HEXSTRING_H */
