/*
 * peas-plugin-store-backend-xml.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 Garrett Regier
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gio/gio.h>
#include <gio/gunixoutputstream.h>

#include <libxml/xmlreader.h>

#ifndef HAVE_SOUP_GNOME
#include <libsoup/soup.h>
#else
#include <libsoup/soup-gnome.h>
#endif

#include "libpeas/peas-dirs.h"
#include "libpeas/peas-plugin-info-priv.h"

#include "peas-gtk-plugin-store-backend-xml.h"

#define PLUGIN_STORE_VERSION "1.0"

#define SIMULATE_PLUGIN_OPS

typedef struct {
  PeasGtkInstallablePluginInfo parent;

  gchar **dependencies;
  gchar *version;
} XMLPluginInfo;

typedef struct {
  PeasGtkInstallablePluginInfo *info;

  gint last_progress;

  PeasGtkPluginStoreProgressCallback progress_callback;
  gpointer progress_user_data;
} AsyncData;

struct _PeasGtkPluginStoreBackendXMLPrivate {
  PeasEngine *engine;
  SoupSession *session;

  /* Cache the plugins to avoid delays when the list is needed */
  GPtrArray *plugins;
};

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE
};

static void peas_gtk_plugin_store_backend_iface_init (PeasGtkPluginStoreBackendInterface *iface);

G_DEFINE_TYPE_EXTENDED (PeasGtkPluginStoreBackendXML,
                        peas_gtk_plugin_store_backend_xml,
                        G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND,
                                               peas_gtk_plugin_store_backend_iface_init))

static void
notify_plugin_list_cb (PeasEngine                   *engine,
                       GParamSpec                   *pspec,
                       PeasGtkPluginStoreBackendXML *xml_backend)
{
  GPtrArray *infos = xml_backend->priv->plugins;
  guint i;

  for (i = 0; i < infos->len; ++i)
    {
      PeasGtkInstallablePluginInfo *info = g_ptr_array_index (infos, i);
      PeasPluginInfo *engine_info;

      engine_info = peas_engine_get_plugin_info (xml_backend->priv->engine,
                                                 peas_gtk_installable_plugin_info_get_module_name (info));

      if (engine_info != NULL)
        info->installed = TRUE;
    }
}

/* Copied from: g_key_file_get_locale_string_list */
static gchar **
get_str_list (xmlChar *str)
{
  int len;

  len = xmlStrlen (str);

  if (str[len - 1] == ';')
    str[len - 1] = '\0';

  return g_strsplit ((gchar *) str, ";", 0);
}

static gboolean
get_file (PeasGtkPluginStoreBackendXML  *xml_backend,
          /*AsyncData                     *data,*/
          const gchar                   *path,
          gchar                        **content,
          goffset                       *length,
          GError                       **error)
{
#if 0
  gchar *url;
  SoupMessage *message;
  SoupBuffer *message_body;

  url = g_strconcat ("http://dl.dropbox.com/u/1125151/plugin-store/plugin-store/",
                     path, NULL);

  message = soup_message_new (SOUP_METHOD_GET, url);
  soup_session_send_message (xml_backend->priv->session, message);

  g_free (url);

  if (!SOUP_STATUS_IS_SUCCESSFUL (message->status_code))
    {
      *content = NULL;
      *length = 0;

      if (error != NULL)
        *error = g_error_new_literal (SOUP_HTTP_ERROR, message->status_code,
                                      message->reason_phrase);

      g_object_unref (message);
      return FALSE;
    }

  *content = (gchar *) message->response_body->data;
  *length = message->response_body->length;

  /*message->response_body->data = NULL;
  message->response_body->length = 0;*/

  message_body = soup_message_body_flatten (message->response_body);
  *content = (gchar *) message_body->data;
  *length = message_body->length;
  g_free (message_body);

  g_object_unref (message);

  return TRUE;

  g_getenv ("PEAS_GTK_PLUGIN_STORE_BACKEND_TESTING");

  if (testing != NULL)
    {
#else
  gchar *filename;

  filename = g_strconcat ("/home/garrett/Dropbox/Public/plugin-store/plugin-store/",
                          path, NULL);

  g_file_get_contents (filename, content, (gsize *) length, error);

  g_free (filename);

  return *content != NULL;
#endif
}

static gchar *
get_real_icon (PeasGtkPluginStoreBackendXML *xml_backend,
               const gchar                  *icon,
               const gchar                  *module_name)
{
  gchar *path;
  gchar *data;
  goffset length;
  gchar *tmp_name;
  gint tmp_fd;
  GOutputStream *stream;
  GError *error = NULL;

  path = g_strdup_printf ("icons/%s/%s", "gedit" /* g_get_prgname () */,
                          module_name);


  if (get_file (xml_backend, path, &data, &length, &error))
    tmp_fd = g_file_open_tmp ("libpeas-store-xml-icon-XXXXXX",
                              &tmp_name, &error);

  g_free (path);

  if (error != NULL)
    {
      g_warning ("Error: %s", error->message);
      g_error_free (error);
      g_free (data);
      return NULL;
    }

  stream = g_unix_output_stream_new (tmp_fd, TRUE);

  g_output_stream_write_all (stream, data, length, NULL, NULL, &error);

  g_object_unref (stream);
  g_free (data);

  if (error != NULL)
    {
      g_warning ("Error: failed to create icon file: %s", error->message);
      g_free (tmp_name);
      g_error_free (error);
      return NULL;
    }

  return tmp_name;
}

static void
free_xml_info (XMLPluginInfo *xml_info)
{
  g_strfreev (xml_info->dependencies);
  g_free (xml_info->version);
}

static PeasGtkInstallablePluginInfo *
process_plugin_node (xmlTextReaderPtr reader)
{
  gsize i;
  XMLPluginInfo *xml_info;
  PeasGtkInstallablePluginInfo *info;
  xmlNodePtr node, child;
  gchar **languages;
  gchar **names;
  gchar **descriptions;

  node = xmlTextReaderExpand (reader);
  if (node == NULL)
    return NULL;

  xml_info = g_new0 (XMLPluginInfo, 1);
  info = (PeasGtkInstallablePluginInfo *) xml_info;
  info->refcount = 1;
  info->available = TRUE;
  info->notify = (GDestroyNotify) free_xml_info;

  languages = (gchar **) g_get_language_names ();

  names = g_new0 (gchar *, g_strv_length (languages));
  descriptions = g_new0 (gchar *, g_strv_length (languages));

  for (child = node->children; child != NULL; child = child->next)
    {
      xmlChar *content, *lang;

      if (child->type != XML_ELEMENT_NODE)
        continue;

      content = xmlNodeGetContent (child);

      if (xmlStrcmp (child->name, BAD_CAST "module") == 0)
        {
          info->module_name = g_strdup ((gchar *) content);
        }
      /*else if (xmlStrcmp (child->name, BAD_CAST "loader") == 0)
        {
          info->loader = g_strdup ((gchar *) content);
        }*/
      else if (xmlStrcmp (child->name, BAD_CAST "name") == 0)
        {
          lang = xmlNodeGetLang (child);

          for (i = 0; i < g_strv_length (languages); ++i)
            {
              if ((lang == NULL && g_strcmp0 (languages[i], "C") == 0) ||
                  xmlStrcmp (lang, BAD_CAST languages[i]) == 0)
                {
                  names[i] = g_strdup ((gchar *) content);
                  break;
                }
            }

          xmlFree (lang);
        }
      else if (xmlStrcmp (child->name, BAD_CAST "description") == 0)
        {
          lang = xmlNodeGetLang (child);

          for (i = 0; i < g_strv_length (languages); ++i)
            {
              if ((lang == NULL && g_strcmp0 (languages[i], "C") == 0) ||
                  xmlStrcmp (lang, BAD_CAST languages[i]) == 0)
                {
                  descriptions[i] = g_strdup ((gchar *) content);
                  break;
                }
            }

          xmlFree (lang);
        }
      else if (xmlStrcmp (child->name, BAD_CAST "icon") == 0)
        {
          info->icon_name = g_strdup ((gchar *) content);
        }
      else if (xmlStrcmp (child->name, BAD_CAST "dependencies") == 0)
        {
          xml_info->dependencies = get_str_list (content);
        }
#if 0
      else if (xmlStrcmp (child->name, BAD_CAST "authors") == 0)
        {
          info->authors = get_str_list (content);
        }
      else if (xmlStrcmp (child->name, BAD_CAST "copyright") == 0)
        {
          info->copyright = g_strdup ((gchar *) content);
        }
      else if (xmlStrcmp (child->name, BAD_CAST "website") == 0)
        {
          info->website = g_strdup ((gchar *) content);
        }
#endif
      else if (xmlStrcmp (child->name, BAD_CAST "version") == 0)
        {
          xml_info->version = g_strdup ((gchar *) content);
        }

      xmlFree (content);
    }

  /* Now skip over the plugin element */
  xmlTextReaderNext (reader);

  /* Use the highest priority translation */
  for (i = 0; i < g_strv_length (languages); ++i)
    {
      if (names[i] != NULL)
        {
          if (info->name == NULL)
            info->name = names[i];
          else
            g_free (names[i]);
        }

      if (descriptions[i] != NULL)
        {
          if (info->desc == NULL)
            info->desc = descriptions[i];
          else
            g_free (descriptions[i]);
        }
    }

  g_free (names);
  g_free (descriptions);

  /*if (peas_gtk_installable_plugin_info_get_authors (info) == NULL)
    info->authors = g_new0 (gchar *, 1);*/

  if (xml_info->dependencies == NULL)
    xml_info->dependencies = g_new0 (gchar *, 1);

  if (peas_gtk_installable_plugin_info_get_module_name (info) == NULL ||
      peas_gtk_installable_plugin_info_get_name (info) == NULL)
    {
      g_warning ("Failed to find module_name, name");
      peas_gtk_installable_plugin_info_unref (info);
      return NULL;
    }

  return info;
}

static GPtrArray *
process_plugins_node (xmlTextReaderPtr reader)
{
  GPtrArray *infos;

  infos = g_ptr_array_new ();
  g_ptr_array_set_free_func (infos, (GDestroyNotify) peas_gtk_installable_plugin_info_unref);

  while (xmlTextReaderRead (reader) == 1)
    {
      xmlChar *name;

      if (xmlTextReaderNodeType (reader) != XML_READER_TYPE_ELEMENT)
        continue;

      name = xmlTextReaderName (reader);

      if (xmlStrcmp (name, BAD_CAST "plugin") == 0)
        {
          PeasGtkInstallablePluginInfo *info;

          info = process_plugin_node (reader);

          if (info != NULL)
            g_ptr_array_add (infos, info);
        }

      xmlFree (name);
    }

  return infos;
}

static GPtrArray *
process_xml_plugins_file (xmlTextReaderPtr reader)
{
  GPtrArray *infos = NULL;
  xmlChar *id = NULL;
  xmlChar *version = NULL;
  gboolean failed = FALSE;

  while (xmlTextReaderRead (reader) == 1 && !failed)
    {
      xmlChar *name;

      if (xmlTextReaderNodeType (reader) != XML_READER_TYPE_ELEMENT)
        continue;

      name = xmlTextReaderName (reader);

      if (xmlStrcmp (name, BAD_CAST "application") == 0)
        {
          id = xmlTextReaderGetAttribute (reader, BAD_CAST "id");
          version = xmlTextReaderGetAttribute (reader, BAD_CAST "version");

          if (xmlStrcmp (id, BAD_CAST "gedit" /* g_get_prgname () */) != 0)
            {
              failed = TRUE;
              g_warning ("Error: program name of '%s' does not match "
                         "application id of '%s'", "gedit" /* g_get_prgname () */, id);
            }
          else if (xmlStrcmp (version, BAD_CAST PLUGIN_STORE_VERSION) != 0)
            {
              failed = TRUE;
              g_warning ("Error: incorrect version of '%s'", version);
            }
        }
      else if (xmlStrcmp (name, BAD_CAST "plugins"))
        {
          infos = process_plugins_node (reader);
        }

      xmlFree (name);
    }

  if (id != NULL)
    xmlFree (id);

  if (version != NULL)
    xmlFree (version);

  return infos;
}

static void
get_plugins_cb (GSimpleAsyncResult        *simple,
                PeasGtkPluginStoreBackend *backend,
                GCancellable              *cancellable)
{
  PeasGtkPluginStoreBackendXML *xml_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (backend);
  gchar *path;
  gchar *content = NULL;
  goffset length;
  GError *error = NULL;
  xmlTextReaderPtr reader;
  GPtrArray *infos;
  guint i;

  path = g_strdup_printf ("%s-%s.xml",
                          "gedit" /* g_get_prgname () */,
                          PLUGIN_STORE_VERSION);

  if (!get_file (xml_backend, path, &content, &length, &error))
    {
      g_simple_async_result_set_from_error (simple, error);
      g_error_free (error);
      goto out;
    }

  reader = xmlReaderForMemory (content, length, path, "UTF-8", 0);

  if (reader == NULL)
    {
      g_simple_async_result_set_error (simple, G_IO_ERROR,
                                       G_IO_ERROR_INVALID_DATA,
                                       "Failed to parse XML");
      goto out;
    }

  infos = process_xml_plugins_file (reader);

  xmlFreeTextReader (reader);

  if (infos == NULL)
    goto out;

  /* For now we do this here so I don't have to pass
   * xml_backend to all of the XML funcs
   */

  for (i = 0; i < infos->len; ++i)
    {
      PeasGtkInstallablePluginInfo *info = g_ptr_array_index (infos, i);

      /* peas_gtk_installable_plugin_info_get_icon_name() returns
       * "libpeas-icon" for NULL icons
       */
      if (info->icon_name != NULL)
        {
          char *real_icon;
          const gchar *module_name;

          module_name = peas_gtk_installable_plugin_info_get_module_name (info);
          real_icon = get_real_icon (xml_backend, info->icon_name, module_name);

          g_free (info->icon_name);

          info->icon_name = real_icon;
        }
    }

  g_simple_async_result_set_op_res_gpointer (simple, infos,
                                             (GDestroyNotify) g_ptr_array_unref);

out:

  g_free (content);
  g_free (path);
}

typedef struct {
  AsyncData *data;
  gint value;
} ProgressData;

static gboolean
progress_in_mainloop (ProgressData *progress)
{
  progress->data->progress_callback (progress->value,
                                     progress->data->progress_user_data);

  return FALSE;
}

static void
send_progress (GIOSchedulerJob *job,
               AsyncData       *data,
               gint             value)
{
  ProgressData *progress;

  if (data->progress_callback == NULL || data->last_progress == value)
    return;

  data->last_progress = value;

  progress = g_new (ProgressData, 1);
  progress->data = data;
  progress->value = value;

  g_io_scheduler_job_send_to_mainloop_async (job,
                                             (GSourceFunc) progress_in_mainloop,
                                             progress, g_free);
}

#if 0
static gboolean
mainloop_barrier (gpointer user_data)
{
  /* Does nothing, but ensures all queued idles before this are run */
  return FALSE;
}
#endif

static gboolean
install_plugin_cb (GIOSchedulerJob    *job,
                   GCancellable       *cancellable,
                   GSimpleAsyncResult *simple)
{
  AsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);
  PeasGtkPluginStoreBackendXML *xml_backend;
  gchar *path;
  gchar *content;
  goffset length;
  GError *error = NULL;

  xml_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (
                    g_async_result_get_source_object (G_ASYNC_RESULT (simple)));

#ifdef SIMULATE_PLUGIN_OPS
    {
      int i;

      /* Downloading file */
      for (i = 0; i < 100; ++i)
        {
          send_progress (job, data, i);
          g_usleep ((4 * G_USEC_PER_SEC) / 100);
        }

      /* Extracting the zip */
      send_progress (job, data, -1);
      g_usleep (3 * G_USEC_PER_SEC);

      goto out;
    }
#endif

  send_progress (job, data, 0);

  path = g_strdup_printf ("plugins/%s/%s.gz",
                          "gedit" /* g_get_prgname () */,
                          peas_gtk_installable_plugin_info_get_module_name (data->info));

  if (!get_file (xml_backend, path, &content, &length, &error))
    {
      g_simple_async_result_set_from_error (simple, error);
      g_error_free (error);
      goto out;
    }

  g_free (path);

  /* Unpack the zip */
  g_simple_async_result_set_error (simple, SOUP_HTTP_ERROR,
                                   404, "This is fake!");

out:

  send_progress (job, data, 100);

  /* GIO's async copy implementation does this */
#if 0
  /* Ensure all progress callbacks are done running in main thread */
  if (data->progress_callback != NULL)
    g_io_scheduler_job_send_to_mainloop (job, mainloop_barrier, NULL, NULL);
#endif

  g_simple_async_result_complete_in_idle (simple);

  return FALSE;
}

static void
delete_recursively (GIOSchedulerJob  *job,
                    AsyncData        *data,
                    GFile            *location,
                    GError          **error)
{
  GFileEnumerator *enumerator;
  GFileInfo *file_info;

  enumerator = g_file_enumerate_children (location,
                                          G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                          G_FILE_QUERY_INFO_NONE,
                                          NULL, error);

  if (*error != NULL)
    goto out;

  while ((file_info = g_file_enumerator_next_file (enumerator, NULL, error)) != NULL)
    {
      GFile *child;

      child = g_file_get_child (location, g_file_info_get_name (file_info));

      if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY)
        delete_recursively (job, data, child, error);
      else
        g_file_delete (child, NULL, error);

      g_object_unref (child);
      g_object_unref (file_info);

      if (*error != NULL)
        goto out;
    }

  g_file_delete (location, NULL, error);

out:

  g_object_unref (enumerator);
}

static gboolean
uninstall_plugin_cb (GIOSchedulerJob    *job,
                     GCancellable       *cancellable,
                     GSimpleAsyncResult *simple)
{
  /*PeasGtkPluginStoreBackendXML *xml_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (backend);*/
  AsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);
  gchar *plugin_store_dir;
  const gchar *module_name;
  gchar *plugin_dir;
  GFile *plugin_store_location;
  GError *error = NULL;

  send_progress (job, data, -1);

  plugin_store_dir = peas_dirs_get_plugin_store_dir ();
  module_name = peas_gtk_installable_plugin_info_get_module_name (data->info);
  plugin_dir = g_build_filename (plugin_store_dir, module_name, NULL);

  plugin_store_location = g_file_new_for_path (plugin_dir);

  g_free (plugin_dir);
  g_free (plugin_store_dir);

#ifdef SIMULATE_PLUGIN_OPS
  g_usleep (5 * G_USEC_PER_SEC);
  goto out;
#endif

  if (!g_file_query_exists (plugin_store_location, NULL))
    {
      g_simple_async_result_set_error (simple, G_FILE_ERROR, G_FILE_ERROR_NOENT,
                                       "Failed to uninstall plugin");
      goto out;
    }

  delete_recursively (job, data, plugin_store_location, &error);

  if (error != NULL)
    {
      g_simple_async_result_set_from_error (simple, error);
      g_error_free (error);
    }

out:

  g_simple_async_result_complete_in_idle (simple);

  g_object_unref (plugin_store_location);
  return FALSE;
}

static void
peas_gtk_plugin_store_backend_xml_init (PeasGtkPluginStoreBackendXML *store)
{
  store->priv = G_TYPE_INSTANCE_GET_PRIVATE (store,
                                             PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_XML,
                                             PeasGtkPluginStoreBackendXMLPrivate);

  store->priv->session = SOUP_SESSION (soup_session_sync_new_with_options (
#ifdef HAVE_SOUP_GNOME
      SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
#endif
      SOUP_SESSION_USER_AGENT, "libpeas-1.0 ",
      SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
      NULL));

  if (g_getenv ("PEAS_DEBUG") != NULL)
    {
      SoupLogger *logger = soup_logger_new (SOUP_LOGGER_LOG_HEADERS, -1);

      soup_session_add_feature (store->priv->session,
                                SOUP_SESSION_FEATURE (logger));

      g_object_unref (logger);
    }
}

static void
peas_gtk_plugin_store_backend_xml_set_property (GObject      *object,
                                                guint         prop_id,
                                                const GValue *value,
                                                GParamSpec   *storepec)
{
  PeasGtkPluginStoreBackendXML *store = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      store->priv->engine = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, storepec);
      break;
    }
}

static void
peas_gtk_plugin_store_backend_xml_get_property (GObject    *object,
                                                guint       prop_id,
                                                GValue     *value,
                                                GParamSpec *storepec)
{
  PeasGtkPluginStoreBackendXML *store = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, store->priv->engine);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, storepec);
      break;
    }
}

static void
peas_gtk_plugin_store_backend_xml_constructed (GObject *object)
{
  PeasGtkPluginStoreBackendXML *store = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (object);

  if (store->priv->engine == NULL)
    store->priv->engine = peas_engine_get_default ();

  g_object_ref (store->priv->engine);

  if (G_OBJECT_CLASS (peas_gtk_plugin_store_backend_xml_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (peas_gtk_plugin_store_backend_xml_parent_class)->constructed (object);
}

static void
peas_gtk_plugin_store_backend_xml_dispose (GObject *object)
{
  PeasGtkPluginStoreBackendXML *store = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (object);

  if (store->priv->engine != NULL)
    {
      g_object_unref (store->priv->engine);
      store->priv->engine = NULL;
    }

  if (store->priv->session != NULL)
    {
      g_object_unref (store->priv->session);
      store->priv->session = NULL;
    }

  if (store->priv->plugins != NULL)
    {
      g_ptr_array_unref (store->priv->plugins);
      store->priv->plugins = NULL;
    }

  G_OBJECT_CLASS (peas_gtk_plugin_store_backend_xml_parent_class)->dispose (object);
}

static void
peas_gtk_plugin_store_backend_xml_class_init (PeasGtkPluginStoreBackendXMLClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_gtk_plugin_store_backend_xml_set_property;
  object_class->get_property = peas_gtk_plugin_store_backend_xml_get_property;
  object_class->constructed = peas_gtk_plugin_store_backend_xml_constructed;
  object_class->dispose = peas_gtk_plugin_store_backend_xml_dispose;

  g_object_class_override_property (object_class, PROP_ENGINE, "engine");

  g_type_class_add_private (object_class, sizeof (PeasGtkPluginStoreBackendXMLPrivate));
}

static void
peas_gtk_plugin_store_backend_xml_get_plugins (PeasGtkPluginStoreBackend *backend,
                                               GCancellable              *cancellable,
                                               GAsyncReadyCallback        callback,
                                               gpointer                   user_data)
{
  PeasGtkPluginStoreBackendXML *xml_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (backend);
  GSimpleAsyncResult *simple;

  simple = g_simple_async_result_new (G_OBJECT (backend), callback, user_data,
                                      peas_gtk_plugin_store_backend_xml_get_plugins);
  g_simple_async_result_set_op_res_gpointer (simple, g_ptr_array_new (),
                                             (GDestroyNotify) g_ptr_array_unref);

  if (xml_backend->priv->plugins != NULL)
    {
      g_simple_async_result_complete_in_idle (simple);
    }
  else
    {
      g_simple_async_result_run_in_thread (simple,
                                           (GSimpleAsyncThreadFunc) get_plugins_cb,
                                           G_PRIORITY_DEFAULT,
                                           cancellable);
    }

  g_object_unref (simple);
}

static GPtrArray *
peas_gtk_plugin_store_backend_xml_get_plugins_finish (PeasGtkPluginStoreBackend  *backend,
                                                      GAsyncResult               *result,
                                                      GError                    **error)
{
  PeasGtkPluginStoreBackendXML *xml_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (backend);
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);
  GPtrArray *infos;

  g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (backend),
                        peas_gtk_plugin_store_backend_xml_get_plugins),
                        NULL);

  if (g_simple_async_result_propagate_error (simple, error))
    return NULL;

  /* If we are just using the cached plugins */
  if (xml_backend->priv->plugins != NULL)
    return g_ptr_array_ref (xml_backend->priv->plugins);

  infos = g_simple_async_result_get_op_res_gpointer (simple);

  /* Must be done before calling notify_plugin_list_cb() */
  xml_backend->priv->plugins = g_ptr_array_ref (infos);

  notify_plugin_list_cb (xml_backend->priv->engine,
                         NULL, xml_backend);

  g_signal_connect (xml_backend->priv->engine,
                    "notify::plugin-list",
                    G_CALLBACK (notify_plugin_list_cb),
                    backend);

  return g_ptr_array_ref (infos);
}

static void
peas_gtk_plugin_store_backend_xml_install_plugin (PeasGtkPluginStoreBackend          *backend,
                                                  PeasGtkInstallablePluginInfo       *info,
                                                  GCancellable                       *cancellable,
                                                  PeasGtkPluginStoreProgressCallback  progress_callback,
                                                  gpointer                            progress_user_data,
                                                  GAsyncReadyCallback                 callback,
                                                  gpointer                            user_data)
{
  GSimpleAsyncResult *simple;
  AsyncData *data;

  g_return_if_fail (peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_installed (info));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_in_use (info));

  info->in_use = TRUE;

  simple = g_simple_async_result_new (G_OBJECT (backend), callback, user_data,
                                      peas_gtk_plugin_store_backend_xml_install_plugin);

  data = g_new0 (AsyncData, 1);
  data->info = info;
  data->progress_callback = progress_callback;
  data->progress_user_data = progress_user_data;
  g_simple_async_result_set_op_res_gpointer (simple, data, g_free);

  g_io_scheduler_push_job ((GIOSchedulerJobFunc) install_plugin_cb,
                           simple, g_object_unref,
                           G_PRIORITY_DEFAULT,
                           cancellable);
}

static PeasGtkInstallablePluginInfo *
peas_gtk_plugin_store_backend_xml_install_plugin_finish (PeasGtkPluginStoreBackend  *backend,
                                                         GAsyncResult               *result,
                                                         GError                    **error)
{
  PeasGtkPluginStoreBackendXML *xml_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (backend);
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);
  AsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);
  const gchar *module_name;
  PeasPluginInfo *engine_info;

  g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (backend),
                        peas_gtk_plugin_store_backend_xml_install_plugin),
                        NULL);

  data->info->in_use = FALSE;

  if (g_simple_async_result_propagate_error (simple, error))
    {
      data->info->available = FALSE;
      return data->info;
    }

#ifdef SIMULATE_PLUGIN_OPS
  goto tmp;
#endif

  peas_engine_rescan_plugins (xml_backend->priv->engine);

  module_name = peas_gtk_installable_plugin_info_get_module_name (data->info);
  engine_info = peas_engine_get_plugin_info (xml_backend->priv->engine,
                                             module_name);

  if (engine_info == NULL ||
      !peas_engine_load_plugin (xml_backend->priv->engine, engine_info))
    {
      /*g_propagate_error (error, g_error_copy (info->error));*/
      data->info->available = FALSE;
      return data->info;
    }

tmp:

  data->info->installed = TRUE;

  return data->info;
}

static void
peas_gtk_plugin_store_backend_xml_uninstall_plugin (PeasGtkPluginStoreBackend          *backend,
                                                    PeasGtkInstallablePluginInfo       *info,
                                                    GCancellable                       *cancellable,
                                                    PeasGtkPluginStoreProgressCallback  progress_callback,
                                                    gpointer                            progress_user_data,
                                                    GAsyncReadyCallback                 callback,
                                                    gpointer                            user_data)
{
  PeasGtkPluginStoreBackendXML *xml_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_XML (backend);
  GSimpleAsyncResult *simple;
  AsyncData *data;
  const gchar *module_name;
  PeasPluginInfo *engine_info;

  g_return_if_fail (peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (peas_gtk_installable_plugin_info_is_installed (info));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_in_use (info));

  info->in_use = TRUE;

  simple = g_simple_async_result_new (G_OBJECT (backend), callback, user_data,
                                      peas_gtk_plugin_store_backend_xml_uninstall_plugin);

  data = g_new0 (AsyncData, 1);
  data->info = info;
  data->progress_callback = progress_callback;
  data->progress_user_data = progress_user_data;
  g_simple_async_result_set_op_res_gpointer (simple, data, g_free);

#ifdef SIMULATE_PLUGIN_OPS
  goto tmp;
#endif

  module_name = peas_gtk_installable_plugin_info_get_module_name (info);
  engine_info = peas_engine_get_plugin_info (xml_backend->priv->engine,
                                             module_name);

  if (engine_info == NULL ||
      !peas_engine_unload_plugin (xml_backend->priv->engine, engine_info))
    {
      /* Oh shit */
      g_simple_async_result_set_error (simple, G_FILE_ERROR, G_FILE_ERROR_NOENT,
                                       "Failed to uninstall plugin");
      g_simple_async_result_complete_in_idle (simple);
      g_object_unref (simple);
    }
  else
    {
      /* Disable the engine's plugin */
      engine_info->available = FALSE;
      /* g_error_set (&engine_info->error, UNINSTALLED); */

tmp:

      /* Even if we fail we are no longer safely installed */
      data->info->installed = FALSE;

      g_io_scheduler_push_job ((GIOSchedulerJobFunc) uninstall_plugin_cb,
                               simple, g_object_unref,
                               G_PRIORITY_DEFAULT,
                               cancellable);
    }
}

static PeasGtkInstallablePluginInfo *
peas_gtk_plugin_store_backend_xml_uninstall_plugin_finish (PeasGtkPluginStoreBackend  *backend,
                                                           GAsyncResult               *result,
                                                           GError                    **error)
{
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);
  AsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);

  g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (backend),
                        peas_gtk_plugin_store_backend_xml_uninstall_plugin),
                        NULL);

  data->info->in_use = FALSE;

  if (!g_simple_async_result_propagate_error (simple, error))
    data->info->available = TRUE;

  return data->info;
}

static void
peas_gtk_plugin_store_backend_iface_init (PeasGtkPluginStoreBackendInterface *iface)
{
  iface->get_plugins = peas_gtk_plugin_store_backend_xml_get_plugins;
  iface->get_plugins_finish = peas_gtk_plugin_store_backend_xml_get_plugins_finish;
  iface->install_plugin = peas_gtk_plugin_store_backend_xml_install_plugin;
  iface->install_plugin_finish = peas_gtk_plugin_store_backend_xml_install_plugin_finish;
  iface->uninstall_plugin = peas_gtk_plugin_store_backend_xml_uninstall_plugin;
  iface->uninstall_plugin_finish = peas_gtk_plugin_store_backend_xml_uninstall_plugin_finish;
}

/**
 * peas_gtk_plugin_store_backend_xml_new:
 * @engine: A #PeasEngine.
 *
 * Creates a new plugin store XML backend.
 *
 * Returns: the new #PeasGtkPluginStoreBackendXML.
 */
PeasGtkPluginStoreBackend *
peas_gtk_plugin_store_backend_xml_new (PeasEngine *engine)
{
  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);

  return PEAS_GTK_PLUGIN_STORE_BACKEND (g_object_new (PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_XML,
                                        "engine", engine,
                                        NULL));
}
