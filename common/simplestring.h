#ifndef SIMPLESTRING_H
#define SIMPLESTRING_H


//! @memberof SimpleString
typedef unsigned int SimpleString__size_t;


//! @ingroup Common
struct SimpleString {
  //! The length of the string, not including the terminating nul byte.
  SimpleString__size_t len;
  //! The character data
  char str[0];
};


#define SIMPLE_STRING(x) \
  ((struct SimpleString *) ((char *) (x) - offsetof(struct SimpleString, str)))
#define SIMPLE_STRING_SIZE(x) ((x) + offsetof(struct SimpleString, str))


#endif /* SIMPLESTRING_H */
