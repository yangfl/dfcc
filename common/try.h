#ifndef TRY_H
#define TRY_H

#include "macro.h"
#include "exception.h"


#define should(test) if likely (test)
#define otherwise ; else
#define check if unlikely (Exception_has(&ex)) break
#define check_syscall(x, msg) if unlikely (!(x)) {SystemCallException(msg); break;}
#define onsuccess if likely (!Exception_has(&ex))


#endif /* TRY_H */
