#include <stdio.h>

#include <glib.h>

#include <macro.h>

#include "../version.h"
#include "sessionid.h"


SessionID Client__get_session_id () {
  static SessionID sid = 0;

  // return cached sid
  if (sid != 0) {
    return sid;
  }

  // generate sid from /etc/machine-id
  do_once {
    FILE *machineid = g_fopen("/etc/machine-id", "r");
    if (machineid == NULL) {
      break;
    }

    char buf[33];
    size_t buf_read = fread(buf, 1, sizeof(buf), machineid);
    if (buf_read < 32) {
      g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
            "/etc/machine-id is supposed to be 32-byte long");
      fclose(machineid);
      break;
    }
    buf[buf_read] = '\0';
    sid = g_str_hash(buf);

    fclose(machineid);
    return sid;
  }

  // fallback
  sid = g_random_int();
  return sid;
}
