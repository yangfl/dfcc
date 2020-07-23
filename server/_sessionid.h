#ifndef DFCC_SERVER_SESSIONID_H
#define DFCC_SERVER_SESSIONID_H

#include <stdint.h>

#include "spawn/_hookedprocessgroupid.h"


/**
 * @memberof Session
 * @brief Session ID
 */
typedef HookedProcessGroupID SessionID;
/**
 * @memberof Session
 * @static
 * @brief Maximum possible Session ID
 */
#define SessionID__MAX UINT32_MAX
/**
 * @memberof Session
 * @static
 * @brief An invaild Session ID
 */
#define SessionID_INVAILD 0


struct SessionManager;


#endif /* DFCC_SERVER_SESSIONID_H */
