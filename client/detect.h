#ifndef DFCC_CLIENT_DETECT_H
#define DFCC_CLIENT_DETECT_H

#include <libsoup/soup.h>
#include <glib.h>

#include "../config/serverurl.h"


bool Client_detect_server (
    SoupSession *session, const struct ServerURL *server_url,
    SoupURI **baseurl);


#endif /* DFCC_CLIENT_DETECT_H */
