#include <errno.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>

#include <libsoup/soup.h>
#include <glib.h>

#include <hexstring.h>
#include <macro.h>
#include <wrapper/file.h>
#include <wrapper/mappedfile.h>
#include <wrapper/soup.h>

#include "../ccargs/ccargs.h"
#include "../config/config.h"
#include "../config/serverurl.h"
#include "../file/hash.h"
#include "../server/job.h"
#include "../server/protocol.h"
#include "../version.h"
#include "sessionid.h"
#include "remote.h"


struct RemoteConnection {
  SoupSession *session;
  SoupCookieJar *cookiejar;
  char sessionid_cookies[sizeof(DFCC_COOKIES_SID) + 2 * sizeof(SessionID)];

  JobID jid;
  SoupURI *baseuri;
  char *rpcurl;
  SoupURI *uploaduri;
  SoupURI *downloaduri;
};


//! @memberof RemoteConnection
static void RemoteConnection__setup_session (
    SoupSession *session, const struct ServerURL *server_url) {
  SoupURI *proxyuri = server_url->proxyurl != NULL ?
    soup_uri_new(server_url->proxyurl) : NULL;

  g_object_set(
    session,
    SOUP_SESSION_PROXY_URI, proxyuri,
    //SOUP_SESSION_SSL_STRICT, !server_url->no_strict_ssl,
    SOUP_SESSION_TIMEOUT, server_url->timeout,
    //SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE
    //SOUP_SESSION_TLS_DATABASE
    //SOUP_SESSION_TLS_INTERACTION
    NULL
  );

  if (proxyuri != NULL) {
    g_object_unref(proxyuri);
  }
}


//! @memberof RemoteConnection
static void RemoteConnection_cleanuri (struct RemoteConnection *conn) {
  if (conn->baseuri != NULL) {
    soup_uri_free(conn->baseuri);
  }
  g_free(conn->rpcurl);
  if (conn->uploaduri != NULL) {
    soup_uri_free(conn->uploaduri);
  }
  if (conn->downloaduri != NULL) {
    soup_uri_free(conn->downloaduri);
  }
}


//! @memberof RemoteConnection
static SoupMessage *RemoteConnection_try_submit (
    struct RemoteConnection *conn, const struct ServerURL *server_url,
    const char rpcmsg[], size_t len, guint *status) {
  // prepare uri
  SoupURI *baseuri = soup_uri_new(server_url->baseurl);
  SoupURI *rpcuri = soup_uri_new_with_base(baseuri, DFCC_RPC_PATH);

  // prepare message
  SoupMessage *msg = soup_message_new_from_uri("POST", rpcuri);
  soup_message_set_request(msg, "text/xml", SOUP_MEMORY_TEMPORARY, rpcmsg, len);

  // prepare session and cookie
  RemoteConnection__setup_session(conn->session, server_url);
  SoupURI *hosturi = soup_uri_copy_host(baseuri);
  soup_cookie_jar_set_cookie(conn->cookiejar, hosturi, conn->sessionid_cookies);
  soup_uri_free(hosturi);

  guint status_ = soup_session_send_message(conn->session, msg);
  if (status_ < 100 || status_ >= 400) {
    // fail, cleanup
    soup_uri_free(baseuri);
  } else {
    // success, fill up uris
    RemoteConnection_cleanuri(conn);
    conn->baseuri = baseuri;
    conn->rpcurl = soup_uri_to_string(rpcuri, FALSE);
    conn->uploaduri = soup_uri_new_with_base(conn->baseuri, DFCC_UPLOAD_PATH);
    conn->downloaduri =
      soup_uri_new_with_base(conn->baseuri, DFCC_DOWNLOAD_PATH);
  }
  soup_uri_free(rpcuri);

  if likely (status != NULL) {
    *status = status_;
  }
  return msg;
}


//! @memberof RemoteConnection
static void RemoteConnection_destroy (struct RemoteConnection *conn) {
  RemoteConnection_cleanuri(conn);

  g_object_unref(conn->session);
}


//! @memberof RemoteConnection
static int RemoteConnection_init (struct RemoteConnection *conn) {
  // setup cookie and session
  conn->cookiejar = soup_cookie_jar_new();
  conn->session = soup_session_new_with_options(
    SOUP_SESSION_ADD_FEATURE, conn->cookiejar,
    SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_SNIFFER,
    SOUP_SESSION_USER_AGENT, DFCC_USER_AGENT,
    NULL);
  g_object_unref(conn->cookiejar);

  // setup logger
  SoupLogger *logger = soup_logger_new(SOUP_LOGGER_LOG_BODY, -1);
  soup_session_add_feature(conn->session, SOUP_SESSION_FEATURE(logger));
  g_object_unref(logger);

  // conn->sessionid_cookies = DFCC_COOKIES_SID + buf2hex(sessionid);
  SessionID sessionid = Client__get_session_id();
  strcpy(conn->sessionid_cookies, DFCC_COOKIES_SID);
  buf2hex(conn->sessionid_cookies + strlen(DFCC_COOKIES_SID),
          &sessionid, sizeof(SessionID));
  conn->sessionid_cookies[sizeof(conn->sessionid_cookies) - 1] = '\0';

  conn->baseuri = NULL;
  conn->rpcurl = NULL;
  conn->uploaduri = NULL;
  conn->downloaduri = NULL;

  return 0;
}


static GVariant *Client_pack_settings (const struct Config *config) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

#define case_type(TYPE, type, func) \
  case G_TYPE_ ## TYPE: { \
    type value = G_STRUCT_MEMBER(type, config, Config__info[i].offset); \
    if (value) { \
      g_variant_builder_add( \
        &builder, "{sv}", \
        Config__info[i].key, g_variant_new_ ## func(value)); \
    } \
    break; \
  }

  for (int i = 0; i < Config__info_n; i++) {
    switch (Config__info[i].type) {
      case_type(BOOLEAN, bool, boolean)
      case_type(CHAR, char, byte)
      case_type(INT, int, int32)
      case_type(INT64, long long, int64)
      default:
        g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
              "Unknown type '%s' for key '%s'",
              g_type_name(Config__info[i].type), Config__info[i].key);
    }
  }

#undef case_type

  return g_variant_builder_end(&builder);
}


static int Client_try_submit (
    struct RemoteConnection *conn,
    const struct ServerURL server_list[], char * const cc_argv[],
    char * const cc_envp[], const char *cc_working_directory,
    GVariant *settings) {
  GError *error = NULL;

  // prepare cc args
  char *xmlrpc_msg = soup_xmlrpc_build_request(
    DFCC_RPC_COMPILE_METHOD_NAME,
    g_variant_new("(^as^assv)", cc_argv, cc_envp,
                  cc_working_directory, settings),
    &error);
  should (error == NULL) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Error when building XML RPC query: %s", error->message);
    g_error_free(error);
    return 1;
  }
  size_t xmlrpc_msg_len = strlen(xmlrpc_msg);

  int ret = 1;

  for (int i = 0; server_list[i].baseurl != NULL; i++) {
    g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
          "Trying server %s", server_list[i].baseurl);

    guint status;
    SoupMessage *msg = RemoteConnection_try_submit(
      conn, server_list + i, xmlrpc_msg, xmlrpc_msg_len, &status);

    if (status < 100 || status >= 400) {
      if (status == SOUP_STATUS_SERVICE_UNAVAILABLE) {
        g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
              "Server %s full", server_list[i].baseurl);
      } else {
        g_log(DFCC_NAME, G_LOG_LEVEL_INFO,
              "Failed to connect to server %s: %s",
              server_list[i].baseurl, msg->reason_phrase);
      }
      g_object_unref(msg);
      continue;
    }

    GVariant *response = soup_xmlrpc_parse_response_e(
      msg, DFCC_RPC_COMPILE_RESPONSE_SIGNATURE, G_LOG_LEVEL_MESSAGE);
    g_object_unref(msg);

    if (response != NULL) {
      g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
            "Selected server %s", server_list[i].baseurl);
      conn->jid = g_variant_get_uint32(response);
      g_variant_unref(response);
      ret = 0;
      break;
    }
  }

  g_free(xmlrpc_msg);
  return ret;
}


static GVariant *Client_query_job (
    struct RemoteConnection *conn, gboolean *finished, GVariant **filelist) {
  GVariant *response = dfcc_session_xmlrpc(
    conn->session, conn->rpcurl, QUERY, conn->jid, FALSE);
  return_if_fail(response != NULL) NULL;
  g_variant_get(response, DFCC_RPC_QUERY_RESPONSE_SIGNATURE,
                finished, filelist);
  return response;
}


static int Client_file_associate (
    struct RemoteConnection *conn, GVariant *filelist) {
  GVariant *response = dfcc_session_xmlrpc_variant(
    conn->session, conn->rpcurl, ASSOCIATE, filelist);
  return_if_fail(response != NULL) 1;
  g_variant_unref(response);
  return 0;
}


static int Client_file_upload (
    struct RemoteConnection *conn, const char *path) {
  GError *error = NULL;
  struct MappedFile m;

  should (MappedFile_init(&m, path, &error) == 0) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_ERROR,
          "Open file '%s' failed: %s", path, error->message);
    g_error_free(error);
    return 1;
  }

  SoupMessage *msg = soup_message_new_from_uri("PUT", conn->uploaduri);
  soup_message_set_request(msg, "application/octet-stream",
                           SOUP_MEMORY_TEMPORARY, m.content, m.length);
  soup_session_send_message(conn->session, msg);

  int ret = 0;
  should (msg->status_code == SOUP_STATUS_OK) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
          "Cannot upload, HTTP code %d", msg->status_code);
    ret = 1;
  }
  g_object_unref(msg);
  return ret;
}


static int Client_file_download (
    struct RemoteConnection *conn, const char *path, FileHash hash) {
  char s_hash[FileHash_STRLEN + 1];
  FileHash_to_string(hash, s_hash);
  SoupURI *fileuri = soup_uri_new_with_base(conn->downloaduri, s_hash);

  SoupMessage *msg = soup_message_new_from_uri("GET", fileuri);
  soup_uri_free(fileuri);

  int ret = 0;
  do_once {
    soup_session_send_message(conn->session, msg);
    should (msg->status_code == SOUP_STATUS_OK) otherwise {
      g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
            "Cannot download %s, HTTP code %d", s_hash, msg->status_code);
      ret = 1;
      break;
    }

    GError *error = NULL;
    FILE *output = g_fopen_e(path, "w", &error);
    should (output != NULL) otherwise {
      g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL, error->message);
      g_error_free(error);
      ret = 1;
      break;
    }

    do_once {
      should (fwrite(
          msg->response_body->data, msg->response_body->length, 1, output
      ) == msg->response_body->length) otherwise {
        g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
              "File write failed: %s", strerror(errno));
        ret = 1;
        break;
      }
    }

    fclose(output);
  }

  g_object_unref(msg);
  return ret;
}


static int Client_remote_missing (
    struct RemoteConnection *conn, GVariant *filelist) {
  return_if_g_variant_not_type(
    filelist, DFCC_RPC_QUERY_RESPONSE_MISSING_SIGNATURE) 1;

  // hash list
  GVariantBuilder builder;
  g_variant_builder_init(&builder,
                         G_VARIANT_TYPE(DFCC_RPC_ASSOCIATE_REQUEST_SIGNATURE));
  bool need_associate = false;

  GError *error = NULL;
  GVariantIter iter;
  char *path;
  FileHash hash;
  for (g_variant_iter_init(&iter, filelist);
       g_variant_iter_loop(&iter, "{st}", &path, &hash);) {
    if (hash == 0) {
      need_associate = true;

      FileHash hash = FileHash_from_file(path, &error);
      should (hash != 0) otherwise {
        g_log(DFCC_NAME, G_LOG_LEVEL_ERROR,
              "Coumpute hash of file '%s' failed: %s", path, error->message);
        g_error_free(error);
        goto loop_error;
      }

      g_variant_builder_add(&builder, "{st}", path, hash);
    } else {
      goto_if_fail(Client_file_upload(conn, path) == 0) loop_error;
    }

named_block(loop_error): {
      g_free(path);
      g_variant_builder_clear(&builder);
      return 1;
    }
  }

  if (need_associate) {
    return_if_fail(
      Client_file_associate(conn, g_variant_builder_end(&builder)) == 0) 1;
  }

  return 0;
}


static int Client_remote_finish (
    struct RemoteConnection *conn, GVariant *filelist,
    struct Result * restrict result) {
  return_if_g_variant_not_type(
    filelist, DFCC_RPC_QUERY_RESPONSE_FINISH_SIGNATURE) 1;

  GVariant *outputs;
  GVariant *info;
  g_variant_get(filelist, DFCC_RPC_QUERY_RESPONSE_FINISH_SIGNATURE,
                &outputs, &info);

  GVariantIter iter;
  char *path;
  FileHash hash;
  for (g_variant_iter_init(&iter, outputs);
       g_variant_iter_loop(&iter, "{st}", &path, &hash);) {
    should (Client_file_download(conn, path, hash) == 0) otherwise {
      g_free(path);
      return 1;
    }
  }

#define case_type(TYPE, type, func) \
  case G_TYPE_ ## TYPE: { \
    type value_ = g_variant_get_ ## func(value); \
    if (value_) { \
      G_STRUCT_MEMBER(type, result, keyinfo->offset) = value_; \
    } \
    break; \
  }

  char *key;
  GVariant *value;
  for (g_variant_iter_init(&iter, info);
       g_variant_iter_loop(&iter, "{st}", &key, &value);) {
    size_t Result__info_n_ = Result__info_n;
    struct StructInfo *keyinfo = lfind(
      key, Result__info, &Result__info_n_,
      sizeof(struct StructInfo), StructInfo_match);
    should (keyinfo != NULL) otherwise {
      g_log(DFCC_NAME, G_LOG_LEVEL_WARNING, "Unknown return info '%s'", key);
      continue;
    }
    switch (keyinfo->type) {
      case_type(BOOLEAN, bool, boolean)
      case_type(CHAR, char, byte)
      case_type(INT, int, int32)
      case_type(INT64, long long, int64)
      default:
        g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
              "Unknown type '%s' for key '%s'",
              g_type_name(keyinfo->type), keyinfo->key);
    }
  }

#undef case_type

  return 0;
}


static int Client_run_remotely_ (
    const struct Config *config, struct Result * restrict result,
    char * const remote_argv[], char * const remote_envp[]) {
  int ret = 0;
  struct RemoteConnection conn;
  RemoteConnection_init(&conn);

  if unlikely (Client_try_submit(
      &conn, config->server_list, remote_argv, remote_envp,
      config->cc_working_directory, Client_pack_settings(config)) != 0) {
    g_log(DFCC_NAME, G_LOG_LEVEL_WARNING, "No server available");
    RemoteConnection_destroy(&conn);
    return 1;
  }

  while (true) {
    gboolean finished;
    GVariant *filelist;

    GVariant *response = Client_query_job(&conn, &finished, &filelist);
    should (response != NULL) otherwise {
      ret = 1;
      break;
    }

    should (finished ?
        Client_remote_missing(&conn, filelist) :
        Client_remote_finish(&conn, filelist, result) == 0) otherwise {
      ret = 1;
      g_variant_unref(response);
      break;
    }

    g_variant_unref(response);
  }

  RemoteConnection_destroy(&conn);
  return ret;
}


int Client_run_remotely (
    struct Config *config, struct Result * restrict result) {
  char **remote_argv = g_strdupv(config->cc_argv);
  char **remote_envp = g_strdupv(config->cc_envp);
  int ret = 1;
  if likely (CCargs_can_run_remotely(&remote_argv, &remote_envp)) {
    ret = Client_run_remotely_(config, result, remote_argv, remote_envp);
  }
  g_strfreev(remote_argv);
  g_strfreev(remote_envp);
  return ret;
}
