#include <stdbool.h>

#include <libsoup/soup.h>
#include <glib/gstdio.h>

#include <macro.h>

#include "../config/config.h"
#include "../file/entry.h"
#include "../file/remoteindex.h"
#include "../protocol.h"
#include "../version.h"
#include "common.h"
#include "context.h"
#include "debug.h"
#include "job.h"
#include "session.h"

/**
 * @addtogroup ServerHandler
 * @{
 */


/**
 * @brief Processes XMLRPC requests of submitting a new compiling job.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param msg a SoupMessage
 * @param param a GVariant
 */
static void Server_rpc_compile (
    struct ServerContext *server_ctx, struct Session *session,
    SoupMessage *msg, GVariant *param) {
  if (!JobTable_try_reserve(&server_ctx->jobtable)) {
    soup_message_set_status(msg, SOUP_STATUS_SERVICE_UNAVAILABLE);
    return;
  }

  char **cc_argv;
  char **cc_envp;
  char *cc_working_directory;
  g_variant_get(param, "(^a&s^a&ss)", &cc_argv, &cc_envp, &cc_working_directory);

  GError *error = NULL;
  struct Job *job = Job_new(
    session->sid, cc_argv, cc_envp, cc_working_directory,
    server_ctx->config->hookfs, server_ctx->config->prgpath, &error);
  should (job != NULL) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_WARNING, "Cannot create job for session %x: %s",
          session->sid, error->message);
    soup_xmlrpc_message_set_fault(msg, 0, error->message);
    g_error_free(error);
  }
  JobTable_insert(&server_ctx->jobtable, job);

  g_free(cc_argv);
  g_free(cc_envp);
  g_free(cc_working_directory);
  g_variant_unref(param);

  should (job != NULL) otherwise return;
  soup_xmlrpc_message_set_response_e(msg, g_variant_new_uint32(job->jid));
}


/**
 * @brief Processes XMLRPC requests of mapping a file path to a hash.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param msg a SoupMessage
 * @param param a GVariant
 */
static void Server_rpc_map (
    struct ServerContext *server_ctx, struct Session *session,
    SoupMessage *msg, GVariant *param) {
  GVariantIter iter;
  gchar *path;
  size_t size;
  struct FileHash hash;
  for (g_variant_iter_init(&iter, param);
       g_variant_iter_next(&iter, "{s(tt)}", &path, &size, &hash.hash);) {
    if unlikely (!g_path_is_absolute(path)) {
      //warn
      continue;
    }

    struct FileEntry *entry = g_malloc(sizeof(struct FileEntry));
    FileEntry_init_with_hash(entry, path, &hash);
    if (!RemoteFileIndex_add(&session->file_index, entry, false)) {
      FileEntry_destroy(entry);
      g_free(entry);
    }
  }
}


/**
 * @brief Processes XMLRPC requests of querying status of compiling jobs.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param msg a SoupMessage
 * @param param a GVariant
 */
static void Server_rpc_query (
    struct ServerContext *server_ctx, struct Session *session,
    SoupMessage *msg, GVariant *param) {
  JobID jid;
  gboolean nonblocking;
  g_variant_get(param, DFCC_RPC_QUERY_REQUEST_SIGNATURE, &jid, &nonblocking);

  if (nonblocking) {
    GRWLockReaderLocker *locker =
      g_rw_lock_reader_locker_new(&server_ctx->jobtable.rwlock);
    struct Job *job = g_hash_table_lookup(server_ctx->jobtable.table, &jid);
    should (job != NULL) otherwise {
      soup_message_set_status(msg, SOUP_STATUS_NOT_FOUND);
      soup_xmlrpc_message_set_fault(msg, 0, "JID not found");
    }
    soup_xmlrpc_message_set_response_e(msg, g_variant_new_boolean(job->p.stopped));
    g_rw_lock_reader_locker_free(locker);
  } else {
    //soup_server_pause_message(server_ctx->server, msg);
  }
}


struct ServerRPCTable {
  const char *name;
  const char *signature;
  void (*handler)(
    struct ServerContext *, struct Session *,
    SoupMessage *, GVariant *);
  void *userdata;
};


static const struct ServerRPCTable rpcs[] = {
  {DFCC_RPC_COMPILE_METHOD_NAME, DFCC_RPC_COMPILE_REQUEST_SIGNATURE, Server_rpc_compile, NULL},
  {DFCC_RPC_MAP_METHOD_NAME, DFCC_RPC_MAP_REQUEST_SIGNATURE, Server_rpc_map, NULL},
  {DFCC_RPC_QUERY_METHOD_NAME, DFCC_RPC_QUERY_REQUEST_SIGNATURE, Server_rpc_query, NULL},
};


static inline int Server__parse_method_name (const char *method_name) {
  for (int i = 0; i < G_N_ELEMENTS(rpcs); i++) {
    if (strcmp(method_name, rpcs[i].name) == 0) {
      return i;
    }
  }
  return -1;
}


void Server_handle_rpc (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg) {
  // only POST allowed
  if unlikely (msg->method != SOUP_METHOD_POST) {
    soup_message_set_status(msg, SOUP_STATUS_METHOD_NOT_ALLOWED);
    return;
  }

  GError *error = NULL;
  do_once {
    SoupXMLRPCParams *xmlrpc_params;
    char *method_name = soup_xmlrpc_parse_request(
      msg->request_body->data, msg->request_body->length, &xmlrpc_params, &error);
    should (method_name != NULL) otherwise {
      soup_xmlrpc_message_log_and_set_fault(
        msg, G_LOG_LEVEL_WARNING, 1,
        "Error when parsing XMLRPC request: %s", error->message);
      break;
    }

    int method_index = Server__parse_method_name(method_name);
    should (method_index != -1) otherwise {
      soup_xmlrpc_message_log_and_set_fault(
        msg, G_LOG_LEVEL_WARNING, 1,
        "Unknown XMLRPC request: %s", method_name);
      g_free(method_name);
      break;
    }
    g_free(method_name);

    GVariant *xmlrpc_variant = soup_xmlrpc_params_parse(
      xmlrpc_params, rpcs[method_index].signature, &error);
    soup_xmlrpc_params_free(xmlrpc_params);
    should (xmlrpc_variant != NULL) otherwise {
      soup_xmlrpc_message_log_and_set_fault(
        msg, G_LOG_LEVEL_WARNING, 1,
        "Error when parsing XMLRPC params: %s", error->message);
      break;
    }

    return rpcs[method_index].handler(server_ctx, session, msg, xmlrpc_variant);
  }

  g_error_free(error);
}


void Server_handle_upload (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg) {
  GError *error = NULL;
  struct FileEntryE *entrye = Cache_index_buf(
    &server_ctx->cache, msg->request_body->data,
    msg->request_body->length, &error);
  should (error == NULL) otherwise {
    soup_xmlrpc_message_set_fault(msg, 1, "err");
  }
  soup_xmlrpc_message_set_response_e(msg, g_variant_new(
    DFCC_RPC_UPLOAD_RESPONSE_SIGNATURE,
    msg->request_body->length, entrye->hash.hash));
  return;
}


void Server_handle_download (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg) {
  do_once {
    const char *s_token =
      path + server_ctx->base_path_len + strlen(DFCC_DOWNLOAD_PATH);
    struct FileHash hash;
    should (FileHash_init_from_string(&hash, s_token) == 0) otherwise break;
    //g_hash_table_lookup(server_ctx->jobs, &hash);
  }

  soup_xmlrpc_message_set_fault(msg, 1, "err");
}


void Server_handle_info (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder,
                         G_VARIANT_TYPE(DFCC_RPC_INFO_RESPONSE_SIGNATURE));
  g_variant_builder_add(&builder, "{sv}", "Server",
                        g_variant_new_string(DFCC_USER_AGENT));
  g_variant_builder_add(&builder, "{sv}", "Nproc-configured",
                        g_variant_new_int32(server_ctx->config->nprocs_conf));
  g_variant_builder_add(&builder, "{sv}", "Nproc-online",
                        g_variant_new_int32(server_ctx->config->nprocs_onln));
  g_variant_builder_add(&builder, "{sv}", "Jobs",
                        g_variant_new_int32(server_ctx->jobtable.max_njob));
  g_variant_builder_add(&builder, "{sv}", "Current-jobs",
                        g_variant_new_int32(server_ctx->jobtable.npending +
                                            server_ctx->jobtable.nrunning));
  soup_xmlrpc_message_set_response_e(msg, g_variant_builder_end(&builder));
}



void Server_handle_homepage (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg) {
  if (msg->method == SOUP_METHOD_GET || msg->method == SOUP_METHOD_HEAD) {
    static const char homepage[] = "\
<html>\r\n\
<body>\r\n\
Hi, " DFCC_NAME " " DFCC_VERSION "\r\n\
</body>\r\n\
</html>\r\n\
";
    soup_message_set_response(msg, "text/html",
                              SOUP_MEMORY_STATIC, homepage, strlen(homepage));
    soup_message_set_status(msg, SOUP_STATUS_OK);
  } else if unlikely (msg->method == SOUP_METHOD_POST) {
    soup_message_set_status(msg, SOUP_STATUS_METHOD_NOT_ALLOWED);
  } else {
    soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
}


/**@}*/
