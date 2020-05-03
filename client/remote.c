#include <errno.h>
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
#include "../protocol.h"
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
    proxyuri = soup_uri_new(server_url->proxyurl) :
    NULL;

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
  soup_uri_free(conn->baseuri);
  g_free(conn->rpcurl);
  soup_uri_free(conn->uploaduri);
  soup_uri_free(conn->downloaduri);
}


//! @memberof RemoteConnection
static SoupMessage *RemoteConnection_try_submit (
    struct RemoteConnection *conn, const struct ServerURL *server_url,
    const char rpcmsg[], size_t len, guint *status) {
  // prepare uri
  soup_uri_free(conn->baseuri);
  conn->baseuri = soup_uri_new(server_url->baseurl);
  SoupURI *rpcuri = soup_uri_new_with_base(conn->baseuri, DFCC_RPC_PATH);

  // prepare message
  SoupMessage *msg = soup_message_new_from_uri("POST", rpcuri);
  soup_message_set_request(msg, "text/xml", SOUP_MEMORY_TEMPORARY, rpcmsg, len);

  // prepare session and cookie
  RemoteConnection__setup_session(conn->session, server_url);
  SoupURI *hosturi = soup_uri_copy_host(conn->baseuri);
  soup_cookie_jar_set_cookie(conn->cookiejar, hosturi, conn->sessionid_cookies);
  soup_uri_free(hosturi);

  guint status_ = soup_session_send_message(conn->session, msg);
  if (status_ < 100 || status_ >= 400) {
    // fail, cleanup
    soup_uri_free(conn->baseuri);
    conn->baseuri = NULL;
  } else {
    // success, fill up uris
    RemoteConnection_cleanuri(conn);
    conn->rpcurl = soup_uri_to_string(rpcuri, FALSE);
    conn->uploaduri = soup_uri_new_with_base(conn->baseuri, DFCC_UPLOAD_PATH);
    conn->downloaduri = soup_uri_new_with_base(conn->baseuri, DFCC_DOWNLOAD_PATH);
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

  g_object_unref(conn->cookiejar);
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


static int Client_try_submit (
    struct RemoteConnection *conn,
    const struct ServerURL server_list[], char * const cc_argv[],
    char * const cc_envp[], const char *cc_working_directory) {
  GError *error = NULL;

  // prepare cc args
  char *xmlrpc_msg = soup_xmlrpc_build_request(
    DFCC_RPC_COMPILE_METHOD_NAME,
    g_variant_new("(^as^ass)", cc_argv, cc_envp, cc_working_directory),
    &error);
  should (error == NULL) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Error when building XML RPC query: %s", error->message);
    g_error_free(error);
    return 1;
  }
  size_t xmlrpc_msg_len = strlen(xmlrpc_msg);

  GVariant *response;
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

    response = soup_xmlrpc_parse_response_e(
      msg, DFCC_RPC_COMPILE_RESPONSE_SIGNATURE, G_LOG_LEVEL_MESSAGE);
    g_object_unref(msg);

    if (response != NULL) {
      g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
            "Selected server %s", server_list[i].baseurl);
      ret = 0;
      break;
    }
  }

  g_free(xmlrpc_msg);

  if (ret == 0) {
    conn->jid = g_variant_get_uint32(response);
    g_variant_unref(response);
  }

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

  bool success;
  const char *reason;
  g_variant_get(response, DFCC_RPC_ASSOCIATE_RESPONSE_SIGNATURE,
                &success, &reason);
  should (success) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
          "Server refused to associate file: %s", reason);
  }
  g_variant_unref(response);
  return !success;
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


static SoupMessage *Client_file_download (
    struct RemoteConnection *conn, SoupURI *downloaduri, FileHash hash) {
  char s_hash[FileHash_STRLEN + 1];
  FileHash_to_string(hash, s_hash);
  SoupURI *fileuri = soup_uri_new_with_base(downloaduri, s_hash);

  SoupMessage *msg = soup_message_new_from_uri("GET", fileuri);
  soup_uri_free(fileuri);

  soup_session_send_message(conn->session, msg);
  should (msg->status_code == SOUP_STATUS_OK) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Cannot download %s, HTTP code %d", s_hash, msg->status_code);
    g_object_unref(msg);
    msg = NULL;
  }

  return msg;
}


static int Client_remote_missing (
    struct RemoteConnection *conn, GVariant *filelist) {
  return_if_g_variant_not_type(
    filelist, DFCC_RPC_QUERY_RESPONSE_MISSING_SIGNATURE) 1;

  // hash list
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE(DFCC_RPC_ASSOCIATE_REQUEST_SIGNATURE));
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
    struct RemoteConnection *conn, GVariant *filelist) {
  return_if_g_variant_not_type(
    filelist, DFCC_RPC_QUERY_RESPONSE_FINISH_SIGNATURE) 1;

/*
  GError *error = NULL;
  GVariantIter iter;
  gchar *path;
  size_t size;
  FileHash hash;
  for (g_variant_iter_init(&iter, filelist);
       g_variant_iter_next(&iter, "{s(tt)}", &path, &size, &hash.hash);) {
    SoupMessage *msg = Client_file_download(conn.session, downloaduri, &hash);
    should (msg != NULL) otherwise {
      g_free(path);
      goto loop_error;
    }

    FILE *output = g_fopen_e(path, "w", &error);
    should (output != NULL) otherwise {
      g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL, error->message);
      g_error_free(error);
      g_free(path);
      goto loop_error;
    }
    should (fwrite(
        msg->response_body->data, msg->response_body->length, 1, output
    ) != msg->response_body->length) otherwise {
      const char *errmsg = strerror(errno);
      g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
            "File write failed: %s", errmsg);
      fclose(output);
      g_object_unref(msg);
      g_free(path);
      goto loop_error;
    }
    fclose(output);

    g_object_unref(msg);
    g_free(path);
  }

  goto break_loop;

loop_error:
    ret = 1;
break_loop:
    g_variant_unref(filelist);
    break;
    */
  return 0;
}


static int Client_run_remotely_ (
    struct Config *config, char * const remote_argv[]) {
  int ret = 0;
  struct RemoteConnection conn;
  RemoteConnection_init(&conn);

  if unlikely (Client_try_submit(
      &conn, config->server_list, remote_argv, config->cc_envp,
      config->cc_working_directory) != 0) {
    g_log(DFCC_NAME, G_LOG_LEVEL_WARNING, "No server available");
    RemoteConnection_destroy(&conn);
    return 1;
  }

  while (1) {
    gboolean finished;
    GVariant *filelist;

    GVariant *response = Client_query_job(&conn, &finished, &filelist);
    should (response != NULL) otherwise {
      ret = 1;
      break;
    }

    should (finished ?
        Client_remote_missing(&conn, filelist) :
        Client_remote_finish(&conn, filelist) == 0) otherwise {
      ret = 1;
      g_variant_unref(response);
      break;
    }

    g_variant_unref(response);
  }

  RemoteConnection_destroy(&conn);
  return ret;
}


int Client_run_remotely (struct Config *config) {
  char **remote_argv = g_strdupv(config->cc_argv);
  int ret = 1;
  if likely (CCargs_can_run_remotely(&remote_argv)) {
    ret = Client_run_remotely_(config, remote_argv);
  }
  g_strfreev(remote_argv);
  return ret;
}
