/*
 * peas-plugin-store-backend-pk.c
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

/* I refuse to believe it! */
#define I_KNOW_THE_PACKAGEKIT_GLIB2_API_IS_SUBJECT_TO_CHANGE
#include <packagekit-glib2/packagekit.h>

#include <libpeas/peas-i18n.h>

#include "libpeas/peas-plugin-info-priv.h"

#include "peas-gtk-plugin-store-backend-pk.h"

typedef struct {
  PeasGtkInstallablePluginInfo parent;

  PkPackage *package;
} PkPluginInfo;

typedef struct {
  PeasGtkPluginStoreProgressCallback callback;
  gpointer user_data;
  gint last_value;
} ProgressData;

typedef struct {
  GCancellable *cancellable;

  GPtrArray *plugins;
} GetPluginsAsyncData;

typedef struct {
  ProgressData progress;
  GCancellable *cancellable;

  PeasGtkInstallablePluginInfo *info;
} UnInstallAsyncData;

struct _PeasGtkPluginStoreBackendPkPrivate {
  PeasEngine *engine;

  PkControl *control;
  PkTask *task;
  PkBitfield filters;

  GError *error;
  GPtrArray *plugins;
};

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE
};

static void peas_gtk_plugin_store_backend_iface_init (PeasGtkPluginStoreBackendInterface *iface);

G_DEFINE_TYPE_EXTENDED (PeasGtkPluginStoreBackendPk,
                        peas_gtk_plugin_store_backend_pk,
                        G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND,
                                               peas_gtk_plugin_store_backend_iface_init))

/* Implementation Notes:
 *
 * We determine if PackageKit is available and what it's backend supports
 * using PkControl.
 *
 * To find the packages that can be installed we find the package that
 * owns the program, then get the packages that require that package
 * and filter them to remove unwanted packages like dev & source packages.
 */

#define NON_FATAL_ERROR non_fatal_error_quark ()

static GQuark
non_fatal_error_quark (void)
{
  static volatile gsize quark = 0;

  if (g_once_init_enter (&quark))
    g_once_init_leave (&quark,
                       g_quark_from_static_string ("non-fatal-error"));

  return quark;
}

/* Copied and modified from gnome-packagekit */
static const gchar *
pk_status_enum_to_progress_message (PkStatusEnum status)
{
  const gchar *text = NULL;

  switch (status)
    {
    case PK_STATUS_ENUM_UNKNOWN:
    case PK_STATUS_ENUM_SETUP:
      text = _("Starting");
      break;
    case PK_STATUS_ENUM_WAIT:
      /* TRANSLATORS: transaction state, the transaction is waiting for another to complete */
      text = _("Waiting in queue");
      break;
    case PK_STATUS_ENUM_REMOVE:
      /* TRANSLATORS: transaction state, removing packages */
      text = _("Uninstalling Plugin");
      break;
    case PK_STATUS_ENUM_DOWNLOAD:
      /* TRANSLATORS: transaction state, downloading package files */
      text = _("Downloading Plugin");
      break;
    case PK_STATUS_ENUM_COPY_FILES:
      /* we are copying package files to prepare to install */
    case PK_STATUS_ENUM_INSTALL:
      /* TRANSLATORS: transaction state, installing packages */
      text = _("Installing Plugin");
      break;
    case PK_STATUS_ENUM_REFRESH_CACHE:
    case PK_STATUS_ENUM_CLEANUP:
    case PK_STATUS_ENUM_ROLLBACK:
    case PK_STATUS_ENUM_TEST_COMMIT:
    case PK_STATUS_ENUM_COMMIT:
    case PK_STATUS_ENUM_SCAN_PROCESS_LIST:
    case PK_STATUS_ENUM_CHECK_EXECUTABLE_FILES:
    case PK_STATUS_ENUM_CHECK_LIBRARIES:
      text = _("Finishing");
      break;
    case PK_STATUS_ENUM_FINISHED:
      text = _("Finished");
      break;
    case PK_STATUS_ENUM_CANCEL:
      /* TRANSLATORS: transaction state, in the process of cancelling */
      text = _("Cancelling");
      break;
    case PK_STATUS_ENUM_WAITING_FOR_LOCK:
      /* TRANSLATORS: transaction state, when we're waiting for the native tools to exit */
      text = _("Waiting for package manager lock");
      break;
    case PK_STATUS_ENUM_WAITING_FOR_AUTH:
      /* TRANSLATORS: waiting for user to type in a password */
      text = _("Waiting for authentication");
      break;
    default:
      /* Show the previous message */
      break;
    }

  return text;
}

static void
pk_plugin_info_destroy_notify (gpointer info)
{
  g_object_unref (((PkPluginInfo *) info)->package);
}

static void
get_plugins_async_data_free (GetPluginsAsyncData *data)
{
  g_ptr_array_unref (data->plugins);
  g_object_unref (data->cancellable);
  g_free (data);
}

static void
progress_proxy_cb (PkProgress     *progress,
                   PkProgressType  type,
                   ProgressData   *data)
{
  const gchar *msg = NULL;

  if (data->callback == NULL)
    return;

  if (type == PK_PROGRESS_TYPE_STATUS)
    {
      PkStatusEnum status;

      g_object_get (progress,
                    "status", &status,
                    NULL);

      msg = pk_status_enum_to_progress_message (status);

      if (status == PK_STATUS_ENUM_FINISHED)
        data->last_value = 100;
    }
  else if (type == PK_PROGRESS_TYPE_PERCENTAGE)
    {
      g_object_get (progress,
                    "percentage", &data->last_value,
                    NULL);
    }

  data->callback (data->last_value, msg, data->user_data);
}

static void
set_error_from_error (GError                            **out_error,
                      GSimpleAsyncResult                 *simple,
                      GError                             *error,
                      PeasGtkInstallablePluginInfoError   error_code)
{
  /* Only pass up that the operation failed but do not make it a fatal error */
  if (error->domain == NON_FATAL_ERROR)
    {
      error->domain = PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR;
    }
  /* Correctly pass up that the operation was cancelled */
  else if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
      g_set_error (out_error,
                   PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR,
                   error_code,
                   error->message);
    }

  g_simple_async_result_set_from_error (simple, error);
  g_error_free (error);

  g_simple_async_result_complete (simple);
  g_object_unref (simple);
}

static void
get_plugins__set_error (GSimpleAsyncResult *simple,
                        GError             *error)
{
  PeasGtkPluginStoreBackendPk *pk_backend;

  pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (
                    g_async_result_get_source_object (G_ASYNC_RESULT (simple)));
  g_object_unref (pk_backend);

  set_error_from_error (&pk_backend->priv->error, simple, error,
                        PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_GET_PLUGINS);
}

static void
un_install_plugin__set_error (GSimpleAsyncResult                *simple,
                              GError                            *error,
                              PeasGtkInstallablePluginInfoError  error_code)
{
  UnInstallAsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);

  set_error_from_error (&data->info->error, simple, error, error_code);
}

#define install_plugin__set_error(s,e) \
  un_install_plugin__set_error ((s), (e), \
                                PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_INSTALL_PLUGIN)

#define uninstall_plugin__set_error(s,e) \
  un_install_plugin__set_error ((s), (e), \
                                PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_UNINSTALL_PLUGIN)

static gboolean
results_set_error (GSimpleAsyncResult                *simple,
                   GError                            *error,
                   PkResults                         *results,
                   PeasGtkInstallablePluginInfoError  error_code)
{
  PkError *pk_error;

  if (error != NULL)
    goto out;

  pk_error = pk_results_get_error_code (results);

  if (pk_error == NULL)
    return FALSE;

  switch (pk_error_get_code (pk_error))
    {
    case PK_ERROR_ENUM_TRANSACTION_CANCELLED:
      {
        GCancellable *cancellable = g_cancellable_new ();

        g_cancellable_cancel (cancellable);
        g_cancellable_set_error_if_cancelled (cancellable, &error);
        g_object_unref (cancellable);

        break;
      }
    case PK_ERROR_ENUM_NO_NETWORK:
    case PK_ERROR_ENUM_NOT_AUTHORIZED:
      g_set_error (&error, NON_FATAL_ERROR,
                   error_code, pk_error_get_details (pk_error));
      break;
    default:
      g_set_error (&error, PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR,
                   error_code, pk_error_get_details (pk_error));
    }

  g_object_unref (pk_error);
  g_object_unref (results);

out:

  if (error_code == PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_GET_PLUGINS)
    get_plugins__set_error (simple, error);
  else
    un_install_plugin__set_error (simple, error, error_code);

  return TRUE;
}

#define get_plugins__results_set_error(s,e,r) \
  results_set_error ((s), (e), (r), \
                     PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_GET_PLUGINS)

#define install_plugin__results_set_error(s,e,r) \
  results_set_error ((s), (e), (r), \
                     PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_INSTALL_PLUGIN)

#define uninstall_plugin__results_set_error(s,e,r) \
  results_set_error ((s), (e), (r), \
                     PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_UNINSTALL_PLUGIN)

static void
get_plugins__get_requires_cb (PkTask             *task,
                              GAsyncResult       *result,
                              GSimpleAsyncResult *simple)
{
  GetPluginsAsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);
  GError *error = NULL;
  PkResults *results;
  GPtrArray *packages;
  guint i;

  results = pk_task_generic_finish (task, result, &error);

  if (get_plugins__results_set_error (simple, error, results))
    return;

  packages = pk_results_get_package_array (results);

  if (packages != NULL)
    {
      g_ptr_array_set_size (data->plugins, packages->len);

      for (i = 0; i < packages->len; ++i)
        {
          PeasGtkInstallablePluginInfo *plugin;
          PkPackage *package = g_ptr_array_index (packages, i);

          plugin = peas_gtk_installable_plugin_info_new (sizeof (PkPluginInfo),
                                                         pk_plugin_info_destroy_notify);

          plugin->module_name = g_strdup (pk_package_get_id (package));
          plugin->name = g_strdup (pk_package_get_name (package));
          plugin->desc = g_strdup (pk_package_get_summary (package));
          /* TODO: plugin->icon_name = ...; */

          if (pk_package_get_info (package) == PK_INFO_ENUM_INSTALLED)
            plugin->installed = TRUE;

          ((PkPluginInfo *) plugin)->package = g_object_ref (package);

          g_ptr_array_index (data->plugins, i) = plugin;
        }

      g_ptr_array_unref (packages);
    }

  g_object_unref (results);

  g_simple_async_result_complete (simple);
  g_object_unref (simple);
}

static void
get_plugins__search_files_cb (PkTask             *task,
                              GAsyncResult       *result,
                              GSimpleAsyncResult *simple)
{
  GetPluginsAsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);
  PeasGtkPluginStoreBackendPk *pk_backend;
  GError *error = NULL;
  PkResults *results;
  GPtrArray *packages;
  PkPackage *package;
  const gchar *package_ids[2] = { NULL, NULL };
  gint i;
  PkBitfield filters = 0;
  const PkFilterEnum wanted_filters[] = {
    PK_FILTER_ENUM_NOT_APPLICATION,
    /*PK_FILTER_ENUM_GUI, ? */
    PK_FILTER_ENUM_VISIBLE,
    PK_FILTER_ENUM_NOT_DEVELOPMENT,
    PK_FILTER_ENUM_NOT_SOURCE,
    PK_FILTER_ENUM_NOT_COLLECTIONS
  };

  pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (
                    g_async_result_get_source_object (G_ASYNC_RESULT (simple)));
  g_object_unref (pk_backend);

  results = pk_task_generic_finish (task, result, &error);

  if (get_plugins__results_set_error (simple, error, results))
    return;

  packages = pk_results_get_package_array (results);

  if (packages == NULL || packages->len != 1)
    {
      g_set_error (&error, PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR,
                   PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_GET_PLUGINS,
                   _("Could not determine which package installed this program"));
      get_plugins__set_error (simple, error);
      goto out;
    }

  package = g_ptr_array_index (packages, 0);
  package_ids[0] = pk_package_get_id (package);

  for (i = 0; i < G_N_ELEMENTS (wanted_filters); ++i)
    {
      if (pk_bitfield_contain (pk_backend->priv->filters, wanted_filters[i]))
        pk_bitfield_add (filters, wanted_filters[i]);
    }

  if (filters == 0)
    filters = pk_bitfield_value (PK_FILTER_ENUM_NONE);
    
  pk_task_get_requires_async (task, filters, (gchar **) package_ids,
                              FALSE /* Recursive? */, data->cancellable,
                              NULL, NULL, /* Progress callback & user_data */
                              (GAsyncReadyCallback) get_plugins__get_requires_cb,
                              simple);


out:

  if (packages != NULL)
    g_ptr_array_unref (packages);

  g_object_unref (results);
}

static void
get_plugins__get_properties_cb (PkControl          *control,
                                GAsyncResult       *result,
                                GSimpleAsyncResult *simple)
{
  GetPluginsAsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);
  PeasGtkPluginStoreBackendPk *pk_backend;
  GError *error = NULL;
  PkBitfield roles;
  PkBitfield filters;
  gchar *files[2] = { NULL, NULL };

  pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (
                    g_async_result_get_source_object (G_ASYNC_RESULT (simple)));
  g_object_unref (pk_backend);

  if (!pk_control_get_properties_finish (control, result, &error))
    {
      get_plugins__set_error (simple, error);
      return;
    }

  g_object_get (control,
                "roles", &roles,
                "filters", &pk_backend->priv->filters,
                NULL);

  if (!pk_bitfield_contain (roles, PK_ROLE_ENUM_SEARCH_FILE) ||
      !pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_REQUIRES) ||
      !pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_FILES))
    {
      g_set_error (&error, PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR,
                   PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_GET_PLUGINS,
                   _("PackageKit's backend does not support required functions"));
      get_plugins__set_error (simple, error);
      return;
    }

  files[0] = g_find_program_in_path ("/usr/bin/gedit");
  /*files[0] = g_find_program_in_path (g_get_prgname ());*/

  if (files[0] == NULL)
    {
      g_set_error (&error, PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR,
                   PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_GET_PLUGINS,
                   _("Could not find program in path"));
      get_plugins__set_error (simple, error);
      return;
    }

  if (pk_bitfield_contain (pk_backend->priv->filters, PK_FILTER_ENUM_INSTALLED))
    filters = pk_bitfield_value (PK_FILTER_ENUM_INSTALLED);
  else
    filters = pk_bitfield_value (PK_FILTER_ENUM_NONE);

  pk_task_search_files_async (pk_backend->priv->task, filters, files,
                              data->cancellable,
                              NULL, NULL, /* Progress callback & user_data */
                              (GAsyncReadyCallback) get_plugins__search_files_cb,
                              simple);
}

static void
install_plugin__install_package_cb (PkTask             *task,
                                    GAsyncResult       *result,
                                    GSimpleAsyncResult *simple)
{
  PeasGtkPluginStoreBackendPk *pk_backend;
  GError *error = NULL;
  PkResults *results;

  pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (
                    g_async_result_get_source_object (G_ASYNC_RESULT (simple)));
  g_object_unref (pk_backend);

  results = pk_task_generic_finish (task, result, &error);

  if (install_plugin__results_set_error (simple, error, results))
    return;

  peas_engine_rescan_plugins (pk_backend->priv->engine);

  g_object_unref (results);

  g_simple_async_result_complete (simple);
  g_object_unref (simple);
}

static void
uninstall_plugin__remove_package_cb (PkTask             *task,
                                     GAsyncResult       *result,
                                     GSimpleAsyncResult *simple)
{
  GError *error = NULL;
  PkResults *results;

  results = pk_task_generic_finish (task, result, &error);

  if (uninstall_plugin__results_set_error (simple, error, results))
    return;

  g_object_unref (results);

  g_simple_async_result_complete (simple);
  g_object_unref (simple);
}

static void
uninstall_plugin__get_files_cb (PkTask             *task,
                                GAsyncResult       *result,
                                GSimpleAsyncResult *simple)
{
  UnInstallAsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);
  PkPluginInfo *pk_info = (PkPluginInfo *) data->info;
  PeasGtkPluginStoreBackendPk *pk_backend;
  GError *error = NULL;
  PkResults *results;
  GPtrArray *files_array;
  PkFiles *pk_files;
  gchar **files;
  guint i;
  const GList *plugins, *l;
  const gchar *package_ids[2] = { NULL, NULL };

  pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (
                    g_async_result_get_source_object (G_ASYNC_RESULT (simple)));
  g_object_unref (pk_backend);

  results = pk_task_generic_finish (task, result, &error);

  if (uninstall_plugin__results_set_error (simple, error, results))
    return;

  files_array = pk_results_get_files_array (results);
  g_warn_if_fail (files_array->len == 1);
  pk_files = g_ptr_array_index (files_array, 0);
  g_object_get (pk_files,
                "files", &files,
                NULL);

  plugins = peas_engine_get_plugin_list (pk_backend->priv->engine);

  for (i = 0; files[i] != NULL; ++i)
    {
      const gchar *file = files[i];

      if (!g_str_has_suffix (file, ".plugin"))
        continue;

      for (l = plugins; l != NULL; l = l->next);
        {
          PeasPluginInfo *engine_info = l->data;

          if (g_strcmp0 (engine_info->filename, file) == 0 &&
              !peas_engine_unload_plugin (pk_backend->priv->engine,
                                          engine_info))
            {
              /* Oh shit... */
              peas_plugin_info_is_available (engine_info, &error);
              uninstall_plugin__set_error (simple, error);
              goto out;
            }
        }
    }

  /* Show a user friendly progress message */
  if (data->progress.callback != NULL)
    data->progress.callback (-1, _("Uninstalling Plugin"),
                             data->progress.user_data);

  package_ids[0] = pk_package_get_id (pk_info->package);

  pk_task_remove_packages_async (task, (gchar **) package_ids,
                                 FALSE, FALSE, data->cancellable,
                                 (PkProgressCallback) progress_proxy_cb, data,
                                 (GAsyncReadyCallback) uninstall_plugin__remove_package_cb,
                                 simple);

out:

  g_strfreev (files);
  g_ptr_array_unref (files_array);
  g_object_unref (results);
}

static void
uninstall_plugin__get_requires_cb (PkTask             *task,
                                   GAsyncResult       *result,
                                   GSimpleAsyncResult *simple)
{
  UnInstallAsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);
  PkPluginInfo *pk_info = (PkPluginInfo *) data->info;
  PeasGtkPluginStoreBackendPk *pk_backend;
  GError *error = NULL;
  PkResults *results;
  GPtrArray *package_ids;
  GPtrArray *packages;
  guint i;

  pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (
                    g_async_result_get_source_object (G_ASYNC_RESULT (simple)));
  g_object_unref (pk_backend);

  results = pk_task_generic_finish (task, result, &error);

  if (uninstall_plugin__results_set_error (simple, error, results))
    return;

  package_ids = g_ptr_array_new ();
  packages = pk_results_get_package_array (results);

  if (packages == NULL)
    {
      g_ptr_array_set_size (package_ids, 2);
    }
  else
    {
      g_ptr_array_set_size (package_ids, packages->len + 2);

      for (i = 0; i < packages->len; ++i)
        {
          PkPackage *package = g_ptr_array_index (packages, i);

          g_ptr_array_index (package_ids, i + 1) = (gchar *) pk_package_get_id (package);
        }

      g_ptr_array_unref (packages);
    }

  g_ptr_array_index (package_ids, 0) = (gchar *) pk_package_get_id (pk_info->package);
  g_ptr_array_index (package_ids, package_ids->len - 1) = NULL;

  pk_task_get_files_async (task, (gchar **) &g_ptr_array_index (package_ids, 0),
                           data->cancellable,
                           NULL, NULL, /* Progress callback & user_data */
                           (GAsyncReadyCallback) uninstall_plugin__get_files_cb,
                           simple);

  g_ptr_array_unref (package_ids);
  g_object_unref (results);
}

static void
peas_gtk_plugin_store_backend_pk_init (PeasGtkPluginStoreBackendPk *pk_backend)
{
  pk_backend->priv = G_TYPE_INSTANCE_GET_PRIVATE (pk_backend,
                                                  PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_PK,
                                                  PeasGtkPluginStoreBackendPkPrivate);

  pk_backend->priv->control = pk_control_new ();

  pk_backend->priv->task = pk_task_new ();
  g_object_set (pk_backend->priv->task,
                "background", FALSE,
                NULL);
}

static void
peas_gtk_plugin_store_backend_pk_set_property (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *paramspec)
{
  PeasGtkPluginStoreBackendPk *pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      pk_backend->priv->engine = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, paramspec);
      break;
    }
}

static void
peas_gtk_plugin_store_backend_pk_get_property (GObject    *object,
                                               guint       prop_id,
                                               GValue     *value,
                                               GParamSpec *paramspec)
{
  PeasGtkPluginStoreBackendPk *pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, pk_backend->priv->engine);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, paramspec);
      break;
    }
}

static void
peas_gtk_plugin_store_backend_pk_constructed (GObject *object)
{
  PeasGtkPluginStoreBackendPk *pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (object);

  if (pk_backend->priv->engine == NULL)
    pk_backend->priv->engine = peas_engine_get_default ();

  g_object_ref (pk_backend->priv->engine);

  G_OBJECT_CLASS (peas_gtk_plugin_store_backend_pk_parent_class)->constructed (object);
}

static void
peas_gtk_plugin_store_backend_pk_dispose (GObject *object)
{
  PeasGtkPluginStoreBackendPk *pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (object);


  if (pk_backend->priv->plugins != NULL)
    {
      g_ptr_array_unref (pk_backend->priv->plugins);
      pk_backend->priv->plugins = NULL;
    }

  g_clear_object (&pk_backend->priv->engine);
  g_clear_object (&pk_backend->priv->control);
  g_clear_object (&pk_backend->priv->task);
  g_clear_error (&pk_backend->priv->error);

  G_OBJECT_CLASS (peas_gtk_plugin_store_backend_pk_parent_class)->dispose (object);
}

static void
void_debug_log_handler (const gchar    *log_domain,
                        GLogLevelFlags  log_level,
                        const gchar    *message,
                        gpointer        user_data)
{
  /* By doing nothing debug messages are not printed */
}

static void
peas_gtk_plugin_store_backend_pk_class_init (PeasGtkPluginStoreBackendPkClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_gtk_plugin_store_backend_pk_set_property;
  object_class->get_property = peas_gtk_plugin_store_backend_pk_get_property;
  object_class->constructed = peas_gtk_plugin_store_backend_pk_constructed;
  object_class->dispose = peas_gtk_plugin_store_backend_pk_dispose;

  g_object_class_override_property (object_class, PROP_ENGINE, "engine");

  g_type_class_add_private (object_class, sizeof (PeasGtkPluginStoreBackendPkPrivate));

  /* Silence PackageKit's debug messages */
  if (g_getenv ("PEAS_DEBUG") == NULL)
    g_log_set_handler ("PackageKit", G_LOG_LEVEL_DEBUG,
                       void_debug_log_handler, NULL);
}

static void
peas_gtk_plugin_store_backend_pk_get_plugins (PeasGtkPluginStoreBackend          *backend,
                                              GCancellable                       *cancellable,
                                              PeasGtkPluginStoreProgressCallback  progress_callback,
                                              gpointer                            progress_user_data,
                                              GAsyncReadyCallback                 callback,
                                              gpointer                            user_data)
{
  PeasGtkPluginStoreBackendPk *pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (backend);
  GSimpleAsyncResult *simple;
  GetPluginsAsyncData *data;

  simple = g_simple_async_result_new (G_OBJECT (backend), callback, user_data,
                                      peas_gtk_plugin_store_backend_pk_get_plugins);

  if (pk_backend->priv->error != NULL || pk_backend->priv->plugins != NULL)
    {
      g_simple_async_result_complete_in_idle (simple);
      g_object_unref (simple);
      return;
    }

  /* Note that because the steps involved in getting the
   * plugins would be confusing and not informational to
   * the user we do not use the progress callback or data.
   */
  data = g_new0 (GetPluginsAsyncData, 1);
  data->plugins = g_ptr_array_new ();
  g_ptr_array_set_free_func (data->plugins,
                             (GDestroyNotify) peas_gtk_installable_plugin_info_unref);

  if (cancellable != NULL)
    data->cancellable = g_object_ref (cancellable);

  g_simple_async_result_set_op_res_gpointer (simple, data,
                                             (GDestroyNotify) get_plugins_async_data_free);

  pk_control_get_properties_async (pk_backend->priv->control, NULL,
                                   (GAsyncReadyCallback) get_plugins__get_properties_cb,
                                   simple);
}

static GPtrArray *
peas_gtk_plugin_store_backend_pk_get_plugins_finish (PeasGtkPluginStoreBackend  *backend,
                                                     GAsyncResult               *result,
                                                     GError                    **error)
{
  PeasGtkPluginStoreBackendPk *pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (backend);
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);

  g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (backend),
                        peas_gtk_plugin_store_backend_pk_get_plugins),
                        NULL);

  /* Fatal error */
  if (pk_backend->priv->error != NULL)
    {
      g_propagate_error (error, g_error_copy (pk_backend->priv->error));
      return NULL;
    }

  /* Non-fatal error */
  if (g_simple_async_result_propagate_error (simple, error))
    return NULL;

  if (pk_backend->priv->plugins == NULL)
    {
      GetPluginsAsyncData *data;

      data = g_simple_async_result_get_op_res_gpointer (simple);
      pk_backend->priv->plugins = g_ptr_array_ref (data->plugins);
    }

  return g_ptr_array_ref (pk_backend->priv->plugins);
}

static void
peas_gtk_plugin_store_backend_pk_install_plugin (PeasGtkPluginStoreBackend          *backend,
                                                 PeasGtkInstallablePluginInfo       *info,
                                                 GCancellable                       *cancellable,
                                                 PeasGtkPluginStoreProgressCallback  progress_callback,
                                                 gpointer                            progress_user_data,
                                                 GAsyncReadyCallback                 callback,
                                                 gpointer                            user_data)
{
  PeasGtkPluginStoreBackendPk *pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (backend);
  PkPluginInfo *pk_info = (PkPluginInfo *) info;
  GSimpleAsyncResult *simple;
  UnInstallAsyncData *data;
  const gchar *package_ids[2] = { NULL, NULL };

  g_return_if_fail (peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_installed (info));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_in_use (info));

  info->in_use = TRUE;

  simple = g_simple_async_result_new (G_OBJECT (backend), callback, user_data,
                                      peas_gtk_plugin_store_backend_pk_install_plugin);

  data = g_new0 (UnInstallAsyncData, 1);
  data->info = info;
  data->progress.callback = progress_callback;
  data->progress.user_data = progress_user_data;
  g_simple_async_result_set_op_res_gpointer (simple, data, g_free);

  package_ids[0] = pk_package_get_id (pk_info->package);

  pk_task_install_packages_async (pk_backend->priv->task,
                                  (gchar **) package_ids, cancellable,
                                  (PkProgressCallback) progress_proxy_cb, data,
                                  (GAsyncReadyCallback) install_plugin__install_package_cb,
                                  simple);
}

static PeasGtkInstallablePluginInfo *
peas_gtk_plugin_store_backend_pk_install_plugin_finish (PeasGtkPluginStoreBackend  *backend,
                                                        GAsyncResult               *result,
                                                        GError                    **error)
{
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);
  UnInstallAsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);

  g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (backend),
                        peas_gtk_plugin_store_backend_pk_install_plugin),
                        NULL);

  data->info->in_use = FALSE;

  if (!g_simple_async_result_propagate_error (simple, error))
    data->info->installed = TRUE;

  return data->info;
}

static void
peas_gtk_plugin_store_backend_pk_uninstall_plugin (PeasGtkPluginStoreBackend          *backend,
                                                   PeasGtkInstallablePluginInfo       *info,
                                                   GCancellable                       *cancellable,
                                                   PeasGtkPluginStoreProgressCallback  progress_callback,
                                                   gpointer                            progress_user_data,
                                                   GAsyncReadyCallback                 callback,
                                                   gpointer                            user_data)
{
  PeasGtkPluginStoreBackendPk *pk_backend = PEAS_GTK_PLUGIN_STORE_BACKEND_PK (backend);
  PkPluginInfo *pk_info = (PkPluginInfo *) info;
  GSimpleAsyncResult *simple;
  UnInstallAsyncData *data;
  const gchar *package_ids[2] = { NULL, NULL };

  g_return_if_fail (peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (peas_gtk_installable_plugin_info_is_installed (info));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_in_use (info));

  info->in_use = TRUE;

  simple = g_simple_async_result_new (G_OBJECT (backend), callback, user_data,
                                      peas_gtk_plugin_store_backend_pk_uninstall_plugin);

  data = g_new0 (UnInstallAsyncData, 1);
  data->info = info;
  data->progress.callback = progress_callback;
  data->progress.user_data = progress_user_data;
  g_simple_async_result_set_op_res_gpointer (simple, data, g_free);

  package_ids[0] = pk_package_get_id (pk_info->package);

  /* Show a user friendly progress message */
  if (progress_callback != NULL)
    progress_callback (-1, _("Unloading Plugin"), progress_user_data);

  pk_task_get_requires_async (pk_backend->priv->task, 0, (gchar **) package_ids,
                              FALSE /* Recursive? */, cancellable,
                              NULL, NULL, /* Progress callback & user_data */
                              (GAsyncReadyCallback) uninstall_plugin__get_requires_cb,
                              simple);
}

static PeasGtkInstallablePluginInfo *
peas_gtk_plugin_store_backend_pk_uninstall_plugin_finish (PeasGtkPluginStoreBackend  *backend,
                                                          GAsyncResult               *result,
                                                          GError                    **error)
{
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);
  UnInstallAsyncData *data = g_simple_async_result_get_op_res_gpointer (simple);

  g_return_val_if_fail (g_simple_async_result_is_valid (result, G_OBJECT (backend),
                        peas_gtk_plugin_store_backend_pk_uninstall_plugin),
                        NULL);

  data->info->in_use = FALSE;

  if (!g_simple_async_result_propagate_error (simple, error))
    data->info->installed = FALSE;

  return data->info;
}

static void
peas_gtk_plugin_store_backend_iface_init (PeasGtkPluginStoreBackendInterface *iface)
{
  iface->get_plugins = peas_gtk_plugin_store_backend_pk_get_plugins;
  iface->get_plugins_finish = peas_gtk_plugin_store_backend_pk_get_plugins_finish;
  iface->install_plugin = peas_gtk_plugin_store_backend_pk_install_plugin;
  iface->install_plugin_finish = peas_gtk_plugin_store_backend_pk_install_plugin_finish;
  iface->uninstall_plugin = peas_gtk_plugin_store_backend_pk_uninstall_plugin;
  iface->uninstall_plugin_finish = peas_gtk_plugin_store_backend_pk_uninstall_plugin_finish;
}

/**
 * peas_gtk_plugin_store_backend_pk_new:
 * @engine: A #PeasEngine.
 *
 * Creates a new plugin store PackageKit backend.
 *
 * Returns: the new #PeasGtkPluginStoreBackendPk.
 */
PeasGtkPluginStoreBackend *
peas_gtk_plugin_store_backend_pk_new (PeasEngine *engine)
{
  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);

  return PEAS_GTK_PLUGIN_STORE_BACKEND (g_object_new (PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_PK,
                                                      "engine", engine,
                                                      NULL));
}
