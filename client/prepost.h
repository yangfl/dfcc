#ifndef DFCC_CLIENT_PREPOST_H
#define DFCC_CLIENT_PREPOST_H
/**
 * @ingroup Client
 * @defgroup PrePost Pre/Post Actions
 * @{
 */

#include "config/config.h"
#include "cc/resultinfo.h"


int Client_pre (struct Config *config);
void Client_post (struct Config *config, struct ResultInfo *result);


/**@}*/

#endif /* DFCC_CLIENT_PREPOST_H */
