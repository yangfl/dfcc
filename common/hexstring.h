#ifndef HEXSTRING_H
#define HEXSTRING_H
/**
 * @ingroup Common
 * @defgroup Hex Hex String
 * @brief Convert between raw data and their hex representation
 * @{
 */


/**
 * @brief Convert a single char to its hex representation.
 *
 * @param c a char
 * @return its hex representation
 */
inline char chr2hex (unsigned char c) {
  if (c >= 0xA) {
    return c + 'a' - 0xA;
  } else {
    return c + '0';
  }
}


/**
 * @brief Convert raw data to its hex representation.
 *
 * @param[out] dst the converted hex representation
 * @param src buffer of data to be convert
 * @param len length of `src`
 */
inline void __attribute__((nonnull)) buf2hex (char *dst, const void *src,
                                              unsigned int len) {
  const unsigned char *src_ = src;
  for (unsigned int i = 0; i < len; i++) {
    dst[2 * i] = chr2hex(src_[i] >> 4);
    dst[2 * i + 1] = chr2hex(src_[i] & 0xF);
  }
}


/**@}*/

#endif /* HEXSTRING_H */
