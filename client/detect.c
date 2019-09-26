#include <libsoup/soup.h>
#include <glib.h>

#include "../config/serverurl.h"
#include "detect.h"


struct RPCResponseFormat {
  const char *name;
  const GVariantType *type;
};


static const struct RPCResponseFormat info_response_format[] = {
  {"Server", G_VARIANT_TYPE_STRING},
  {"Nproc-configured", G_VARIANT_TYPE_INT32},
  {"Nproc-online", G_VARIANT_TYPE_INT32},
  {"Jobs", G_VARIANT_TYPE_INT32},
  {"Current-jobs", G_VARIANT_TYPE_INT32},
};


// purly informative, the real dispatch follows a try-fail approach.
bool Client_detect_server (
    SoupSession *session, const struct ServerURL *server_url,
    SoupURI **baseurl) {
  Client_setup_session(session, server_url);
  SoupURI *url = soup_uri_new(server_url->baseurl);

  SoupURI *infourl = soup_uri_new_with_base(url, DFCC_INFO_PATH);
  SoupMessage *msg = soup_message_new_from_uri("GET", infourl);
  soup_uri_free(infourl);
  soup_session_send_message(session, msg);

  do_once {
    if (msg->status_code != SOUP_STATUS_OK) {
      g_log(DFCC_NAME, G_LOG_LEVEL_INFO,
            "Server %s not responsing", server_url->baseurl);
      g_object_unref(msg);
      break;
    }

    GError *error = NULL;
    GVariant *server_info = soup_xmlrpc_parse_response(
      msg->response_body->data, msg->response_body->length,
      DFCC_RPC_INFO_RESPONSE_SIGNATURE, &error);
    g_object_unref(msg);

    should (server_info != NULL) otherwise {
      if (error->domain == SOUP_XMLRPC_FAULT) {
        g_log(DFCC_NAME, G_LOG_LEVEL_MESSAGE,
              "Server %s report fault: %d %s",
              server_url->baseurl, error->code, error->message);
      } else {
        g_log(DFCC_NAME, G_LOG_LEVEL_MESSAGE,
              "Error when parsing response from server %s: %s",
              server_url->baseurl, error->message);
      }
      g_error_free(error);
      break;
    }

    int jobs;

    GVariantIter iter;
    gchar *key;
    GVariant *value;
    for (g_variant_iter_init(&iter, server_info);
         g_variant_iter_next(&iter, "{sv}", &key, &value);) {
      const GVariantType *value_type = g_variant_get_type(value);

      int i;
      for (i = 0; i < G_N_ELEMENTS(info_response_format); i++) {
        if (strcmp(key, info_response_format[i].name) == 0) {
          should (g_variant_type_is_subtype_of(
              value_type, info_response_format[i].type)) otherwise {
            gchar *expected_value_type_string =
              g_variant_type_dup_string(info_response_format[i].type);
            g_log(DFCC_NAME, G_LOG_LEVEL_MESSAGE,
                  "Item '%s' from server %s should have type '%s', got '%s'",
                  key, server_url->baseurl,
                  expected_value_type_string, g_variant_get_type_string(value));
            g_free(expected_value_type_string);
            goto unexpected;
          }
          break;
        }
      }

      switch (i) {
        case 0: {
          const gchar *server_version = g_variant_get_string(value, NULL);
          should (strscmp(server_version, DFCC_NAME "/") == 0) otherwise {
            g_log(DFCC_NAME, G_LOG_LEVEL_MESSAGE, "Unexpected server %s: %s",
                  server_url->baseurl, server_version);
            goto unexpected;
          }
          g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG, "Server: %s", server_version);
          break;
        }
        case 1: {
          int nprocs_conf = g_variant_get_int32(value);
          g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
                "Server has %d core(s) configured", nprocs_conf);
          break;
        }
        case 2: {
          int nprocs_onln = g_variant_get_int32(value);
          g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
                "Server has %d core(s) online", nprocs_onln);
          break;
        }
        case 3: {
          jobs = g_variant_get_int32(value);
          g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
                "Server can has %d job(s)", jobs);
          break;
        }
        case 4: {
          int current_jobs = g_variant_get_int32(value);
          g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
                "Server currently has %d job(s)", current_jobs);
          if (current_jobs >= jobs) {
            g_log(DFCC_NAME, G_LOG_LEVEL_INFO,
                  "Server %s is full", server_url->baseurl);
            goto unexpected;
          }
          break;
        }
        default:
          g_log(DFCC_NAME, G_LOG_LEVEL_MESSAGE, "Item '%s' has type '%s'", key,
                g_variant_get_type_string (value));
      }

      g_variant_unref(value);
      g_free(key);
      continue;

unexpected:
      g_variant_unref(value);
      g_free(key);
      goto fail;
    }

    if (baseurl != NULL) {
      *baseurl = url;
    } else {
      soup_uri_free(url);
    }

    return true;
  }

fail:
  soup_uri_free(url);
  return false;
}
