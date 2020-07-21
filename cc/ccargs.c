#include <stdbool.h>

#include <glib.h>

#include "ccargs.h"


bool CC_can_run_remotely (char **cc_argv[], char **cc_envp[]) {
  static const char *variables[] = {
    "CLUTTER_IM_MODULE", "COLORTERM", "DBUS_SESSION_BUS_ADDRESS",
    "DESKTOP_SESSION", "DISPLAY", "GDM_LANG", "GDMSESSION", "GPG_AGENT_INFO",
    "GTK_IM_MODULE", "GTK_OVERLAY_SCROLLING", "HOME", "LC_CTYPE", "LESS",
    "LOGNAME", "LS_COLORS", "LSCOLORS", "MATE_DESKTOP_SESSION_ID", "PAGER",
    "PATH", "QT4_IM_MODULE", "QT_ACCESSIBILITY", "QT_AUTO_SCREEN_SCALE_FACTOR",
    "QT_IM_MODULE", "QT_SCALE_FACTOR", "SESSION_MANAGER", "SHELL", "SHLVL",
    "SSH_AGENT_PID", "SSH_AUTH_SOCK", "USER", "VTE_VERSION", "WINDOWID",
    "XAUTHORITY", "XDG_CURRENT_DESKTOP", "XDG_DATA_DIRS",
    "XDG_GREETER_DATA_DIR", "XDG_RUNTIME_DIR", "XDG_SEAT", "XDG_SEAT_PATH",
    "XDG_SESSION_CLASS", "XDG_SESSION_DESKTOP", "XDG_SESSION_ID",
    "XDG_SESSION_PATH", "XDG_SESSION_TYPE", "XDG_VTNR", "XMODIFIERS", "ZSH"
  };
  for (int i = 0; i < G_N_ELEMENTS(variables); i++) {
    *cc_envp = g_environ_unsetenv(*cc_envp, variables[i]);
  }
  return true;
}
