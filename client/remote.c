#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include <libsoup/soup.h>
#include <glib.h>

#include <hexstring.h>
#include <macro.h>

#include "../config/config.h"
#include "../config/serverurl.h"
#include "../file/common.h"
#include "../server/job.h"
#include "../protocol.h"
#include "../version.h"
#include "sessionid.h"
#include "remote.h"


static GVariant *soup_xmlrpc_parse_response_e (
    SoupMessage *msg, const char *signature, GLogLevelFlags log_level) {
  GError *error = NULL;
  GVariant *response = soup_xmlrpc_parse_response(
    msg->response_body->data, msg->response_body->length,
    signature, &error);

  should (response != NULL) otherwise {
    g_log(DFCC_NAME, log_level,
          error->domain == SOUP_XMLRPC_ERROR ?
            "Error when parsing the response: %s" :
            "Error when connecting to server: %s",
          error->message);
    g_error_free(error);
  }

  return response;
}


static void Client_setup_session (
    SoupSession *session, const struct ServerURL *server_url) {
  SoupURI *proxyurl;
  if (server_url->proxyurl == NULL) {
    proxyurl = NULL;
  } else {
    proxyurl = soup_uri_new(server_url->proxyurl);
  }

  g_object_set(
    session,
    SOUP_SESSION_PROXY_URI, proxyurl,
    //SOUP_SESSION_SSL_STRICT, !server_url->no_strict_ssl,
    SOUP_SESSION_TIMEOUT, server_url->timeout,
    //SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE
    //SOUP_SESSION_TLS_DATABASE
    //SOUP_SESSION_TLS_INTERACTION
    NULL
  );

  if (proxyurl != NULL) {
    soup_uri_free(proxyurl);
  }
}


static int Client_try_submit_job (
    SoupSession *session, SoupCookieJar *cookiejar,
    const struct ServerURL server_list[], char * const cc_argv[],
    char * const cc_envp[], const char *cc_working_directory,
    JobID *jid, SoupURI **baseuri, SoupURI **rpcuri) {
  GError *error = NULL;

  char *xmlrpc_msg = soup_xmlrpc_build_request(
    DFCC_RPC_COMPILE_METHOD_NAME,
    g_variant_new("(^as^ass)", cc_argv, cc_envp, cc_working_directory),
    &error);
  should (error == NULL) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_MESSAGE,
          "Error when building XML RPC query: %s", error->message);
    g_error_free(error);
    return 1;
  }
  gsize xmlrpc_msg_len = strlen(xmlrpc_msg);

  SessionID sessionid = Client__get_session_id();
  char sessionid_cookies[sizeof(DFCC_COOKIES_SID) + 2 * sizeof(SessionID)] =
    DFCC_COOKIES_SID;
  buf2hex(sessionid_cookies + strlen(DFCC_COOKIES_SID),
          &sessionid, sizeof(SessionID));
  sessionid_cookies[sizeof(sessionid_cookies) - 1] = '\0';

  SoupURI *baseuri_;
  SoupURI *rpcuri_;
  GVariant *response;
  int ret = 1;

  for (int i = 0; server_list[i].baseurl != NULL; i++) {
    g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
          "Trying server %s", server_list[i].baseurl);

    baseuri_ = soup_uri_new(server_list[i].baseurl);
    rpcuri_ = soup_uri_new_with_base(baseuri_, DFCC_RPC_PATH);

    SoupMessage *msg = soup_message_new_from_uri("POST", rpcuri_);
    soup_message_set_request(msg, "text/xml", SOUP_MEMORY_TEMPORARY,
                             xmlrpc_msg, xmlrpc_msg_len);

    Client_setup_session(session, &server_list[i]);
    SoupURI *hosturi = soup_uri_copy_host(baseuri_);
    soup_cookie_jar_set_cookie(cookiejar, hosturi, sessionid_cookies);
    soup_uri_free(hosturi);

    guint status = soup_session_send_message(session, msg);
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
      goto try_next;
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

try_next:
    soup_uri_free(rpcuri_);
    soup_uri_free(baseuri_);
  }

  g_free(xmlrpc_msg);

  if (ret == 0) {
    *jid = g_variant_get_uint32(response);
    g_variant_unref(response);

    if likely (baseuri != NULL) {
      *baseuri = baseuri_;
    } else {
      soup_uri_free(baseuri_);
    }
    if likely (rpcuri != NULL) {
      *rpcuri = rpcuri_;
    } else {
      soup_uri_free(rpcuri_);
    }
  }

  return ret;
}


static int Client_query_job (
    SoupSession *session, const char *rpcurl, JobID jid,
    gboolean *finished, GVariant **filelist) {
  GError *error = NULL;
  SoupMessage *msg = soup_xmlrpc_message_new(
    rpcurl, DFCC_RPC_QUERY_METHOD_NAME,
    g_variant_new(DFCC_RPC_QUERY_REQUEST_SIGNATURE, jid, FALSE), &error);
  should (msg != NULL) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Error when building XML RPC query: %s", error->message);
    g_error_free(error);
    return 1;
  }

  soup_session_send_message(session, msg);

  GVariant *response = soup_xmlrpc_parse_response_e(
    msg, DFCC_RPC_QUERY_RESPONSE_SIGNATURE, G_LOG_LEVEL_CRITICAL);
  g_object_unref(msg);

  should (response != NULL) otherwise {
    return 1;
  }

  g_variant_get(response, DFCC_RPC_QUERY_RESPONSE_SIGNATURE,
                finished, filelist);
  return 0;
}


static int Client_upload (
    SoupSession *session, SoupURI *uploaduri, const char *buf, size_t len) {
  SoupMessage *msg = soup_message_new_from_uri("PUT", uploaduri);
  soup_message_set_request(msg, "application/octet-stream",
                           SOUP_MEMORY_TEMPORARY, buf, len);
  soup_session_send_message(session, msg);
  if (msg->status_code != SOUP_STATUS_OK) {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Cannot upload, HTTP code %d", msg->status_code);
    g_object_unref(msg);
    return 1;
  }
  g_object_unref(msg);
  return 0;
}


static SoupMessage *Client_download (
    SoupSession *session, SoupURI *downloaduri, struct FileHash *hash) {
  char s_hash[FileHash_STRLEN + 1];
  FileHash_to_string(hash, s_hash);
  SoupURI *fileuri = soup_uri_new_with_base(downloaduri, s_hash);

  SoupMessage *msg = soup_message_new_from_uri("GET", fileuri);
  soup_uri_free(fileuri);

  soup_session_send_message(session, msg);
  if (msg->status_code != SOUP_STATUS_OK) {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Cannot download %s, HTTP code %d", s_hash, msg->status_code);
    g_object_unref(msg);
    msg = NULL;
  }

  return msg;
}


int Client_run_remotely (struct Config *config) {
  int ret = 0;

  SoupCookieJar *cookiejar = soup_cookie_jar_new();
  SoupSession *session = soup_session_new_with_options(
    SOUP_SESSION_ADD_FEATURE, cookiejar,
    SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_SNIFFER,
    SOUP_SESSION_USER_AGENT, DFCC_USER_AGENT,
    NULL);

  // setup logger
  SoupLogger *logger = soup_logger_new(SOUP_LOGGER_LOG_BODY, -1);
  soup_session_add_feature(session, SOUP_SESSION_FEATURE(logger));
  g_object_unref(logger);

  SoupURI *baseuri;
  SoupURI *rpcuri;
  JobID jid;
  if unlikely (Client_try_submit_job(
      session, cookiejar, config->server_list, config->cc_argv, config->cc_envp,
      config->cc_working_directory, &jid, &baseuri, &rpcuri) != 0) {
    g_log(DFCC_NAME, G_LOG_LEVEL_WARNING, "No server available");
    g_object_unref(session);
    return 1;
  }

  char *rpcurl = soup_uri_to_string(rpcuri, FALSE);
  soup_uri_free(rpcuri);

  SoupURI *uploaduri = soup_uri_new_with_base(baseuri, DFCC_UPLOAD_PATH);
  SoupURI *downloaduri = soup_uri_new_with_base(baseuri, DFCC_DOWNLOAD_PATH);

  while (1) {
    gboolean finished;
    GVariant *filelist;

    should (Client_query_job(
        session, rpcurl, jid, &finished, &filelist) == 0) otherwise {
      ret = 1;
      break;
    }

    GError *error = NULL;

    if (finished) {
      // download file
      GVariantIter iter;
      gchar *path;
      size_t size;
      struct FileHash hash;
      for (g_variant_iter_init(&iter, filelist);
           g_variant_iter_next(&iter, "{s(tt)}", &path, &size, &hash.hash);) {
        SoupMessage *msg = Client_download(session, downloaduri, &hash);
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
    } else {
      gsize missings_len;
      const gchar **missings = g_variant_get_strv(filelist, &missings_len);
      for (int i = 0; i < missings_len; i++) {
        GMappedFile *mappedfile = g_mapped_file_new(missings[i], FALSE, &error);
        should (mappedfile != NULL) otherwise {
          g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
                "File mapping failed: %s", error->message);
          g_error_free(error);
          g_free(missings);
          goto loop_error;
        }
        should (Client_upload(
            session, uploaduri, g_mapped_file_get_contents(mappedfile),
            g_mapped_file_get_length(mappedfile))) otherwise {
          g_free(missings);
          goto loop_error;
        }
        g_mapped_file_unref(mappedfile);
      }
      g_free(missings);
      g_variant_unref(filelist);
    }

    continue;
loop_error:
    ret = 1;
break_loop:
    g_variant_unref(filelist);
    break;
  }

  soup_uri_free(baseuri);
  g_free(rpcurl);
  soup_uri_free(uploaduri);
  soup_uri_free(downloaduri);
  g_object_unref(session);
  return ret;
}
