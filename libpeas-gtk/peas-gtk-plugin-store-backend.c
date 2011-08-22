/*
 * peas-gtk-plugin-store-backend.c
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

#include <libpeas/peas-engine.h>

#include "libpeas/peas-plugin-info-priv.h"

#include "peas-gtk-plugin-store-backend.h"

G_DEFINE_INTERFACE(PeasGtkPluginStoreBackend, peas_gtk_plugin_store_backend, G_TYPE_OBJECT)

static void
peas_gtk_plugin_store_backend_default_init (PeasGtkPluginStoreBackendInterface *iface)
{
  static gsize initialized = 0;

  if (!g_once_init_enter (&initialized))
    return;

  /**
   * PeasGtkPluginStoreBackend:engine:
   *
   * The #PeasEngine the backend is attached to.
   */
  g_object_interface_install_property (iface,
                                       g_param_spec_object ("engine",
                                                            "Engine",
                                                            "The PeasEngine the backend is attached to",
                                                            PEAS_TYPE_ENGINE,
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_STATIC_STRINGS));

  g_once_init_leave (&initialized, 1);
}

/**
 * peas_gtk_plugin_store_backend_get_plugins:
 * @backend: A #PeasGtkPluginStoreBackend.
 * @cancellable: A #GCancellable, or %NULL.
 * @progress_callback: A #PeasGtkPluginStoreProgress that will be called
 *  when progress has been made.
 * @progress_user_data: The data to pass to @progress_callback.
 * @callback: (scope async): A #GAsyncReadyCallback to call when done.
 * @user_data: The data to pass to @callback.
 *
 * Asynchronously gets the available plugins for @backend and calls @callback.
 */
void
peas_gtk_plugin_store_backend_get_plugins (PeasGtkPluginStoreBackend          *backend,
                                           GCancellable                       *cancellable,
                                           PeasGtkPluginStoreProgressCallback  progress_callback,
                                           gpointer                            progress_user_data,
                                           GAsyncReadyCallback                 callback,
                                           gpointer                            user_data)
{
  PeasGtkPluginStoreBackendInterface *iface;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE_BACKEND (backend));
  g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (callback != NULL);

  iface = PEAS_GTK_PLUGIN_STORE_BACKEND_GET_IFACE (backend);

  g_assert (iface->get_plugins != NULL);
  iface->get_plugins (backend, cancellable, progress_callback,
                      progress_user_data, callback, user_data);
}

/**
 * peas_gtk_plugin_store_backend_get_plugins_finish:
 * @backend: A #PeasGtkPluginStoreBackend.
 * @result: A #GAsyncResult obtained from the #GAsyncReadyCallback
 *          passed to peas_gtk_plugin_store_backend_get_plugins().
 * @error: Return location for error or %NULL.
 *
 * Finishes an operation started by peas_gtk_plugin_store_backend_get_plugins().
 *
 * Returns: (element-type Peas.PluginInfo): %NULL if error is set
 * otherwise a #GPtrArray. Free with g_ptr_array_unref().
 */
GPtrArray *
peas_gtk_plugin_store_backend_get_plugins_finish (PeasGtkPluginStoreBackend  *backend,
                                                  GAsyncResult               *result,
                                                  GError                    **error)
{
  PeasGtkPluginStoreBackendInterface *iface;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_STORE_BACKEND (backend), NULL);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  iface = PEAS_GTK_PLUGIN_STORE_BACKEND_GET_IFACE (backend);

  g_assert (iface->get_plugins_finish != NULL);
  return iface->get_plugins_finish (backend, result, error);
}

/**
 * peas_gtk_plugin_store_backend_install_plugin:
 * @backend: A #PeasGtkPluginStoreBackend.
 * @info: A #PeasGtkInstallablePluginInfo.
 * @cancellable: A #GCancellable, or %NULL.
 * @progress_callback: A #PeasGtkPluginStoreProgress that will be called
 *  when progress has been made.
 * @progress_user_data: The data to pass to @progress_callback.
 * @callback: (scope async): A #GAsyncReadyCallback to call when done.
 * @user_data: The data to pass to @callback.
 *
 * Asynchronously installs @info and calls @callback.
 *
 * Note: this does not load the plugins.
 */
void
peas_gtk_plugin_store_backend_install_plugin (PeasGtkPluginStoreBackend          *backend,
                                              PeasGtkInstallablePluginInfo       *info,
                                              GCancellable                       *cancellable,
                                              PeasGtkPluginStoreProgressCallback  progress_callback,
                                              gpointer                            progress_user_data,
                                              GAsyncReadyCallback                 callback,
                                              gpointer                            user_data)
{
  PeasGtkPluginStoreBackendInterface *iface;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE_BACKEND (backend));
  g_return_if_fail (info != NULL && peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (progress_callback != NULL || progress_user_data == NULL);
  g_return_if_fail (callback != NULL);

  iface = PEAS_GTK_PLUGIN_STORE_BACKEND_GET_IFACE (backend);

  g_assert (iface->install_plugin != NULL);
  iface->install_plugin (backend, info, cancellable, progress_callback,
                         progress_user_data, callback, user_data);
}

/**
 * peas_gtk_plugin_store_backend_install_plugin_finish:
 * @backend: A #PeasGtkPluginStoreBackend.
 * @result: A #GAsyncResult obtained from the #GAsyncReadyCallback
 *          passed to peas_gtk_plugin_store_backend_install_plugin().
 * @error: Return location for error or %NULL.
 *
 * Finishes an operation started by peas_gtk_plugin_store_backend_install_plugin().
 *
 * Returns: The #PeasGtkInstallablePluginInfo being installed, even if an error occured.
 */
PeasGtkInstallablePluginInfo *
peas_gtk_plugin_store_backend_install_plugin_finish (PeasGtkPluginStoreBackend  *backend,
                                                     GAsyncResult               *result,
                                                     GError                    **error)
{
  PeasGtkPluginStoreBackendInterface *iface;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_STORE_BACKEND (backend), NULL);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  iface = PEAS_GTK_PLUGIN_STORE_BACKEND_GET_IFACE (backend);

  g_assert (iface->install_plugin_finish != NULL);
  return iface->install_plugin_finish (backend, result, error);
}

/**
 * peas_gtk_plugin_store_backend_uninstall_plugin:
 * @backend: A #PeasGtkPluginStoreBackend.
 * @info: A #PeasGtkInstallablePluginInfo.
 * @cancellable: A #GCancellable, or %NULL.
 * @progress_callback: A #PeasGtkPluginStoreProgress that will be called
 *  when progress has been made.
 * @progress_user_data: The data to pass to @progress_callback.
 * @callback: (scope async): A #GAsyncReadyCallback to call when done.
 * @user_data: The data to pass to @callback.
 *
 * Asynchronously uninstalls @info and calls @callback.
 */
void
peas_gtk_plugin_store_backend_uninstall_plugin (PeasGtkPluginStoreBackend          *backend,
                                                PeasGtkInstallablePluginInfo       *info,
                                                GCancellable                       *cancellable,
                                                PeasGtkPluginStoreProgressCallback  progress_callback,
                                                gpointer                            progress_user_data,
                                                GAsyncReadyCallback                 callback,
                                                gpointer                            user_data)
{
  PeasGtkPluginStoreBackendInterface *iface;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE_BACKEND (backend));
  g_return_if_fail (info != NULL && peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (progress_callback != NULL || progress_user_data == NULL);
  g_return_if_fail (callback != NULL);

  iface = PEAS_GTK_PLUGIN_STORE_BACKEND_GET_IFACE (backend);

  g_assert (iface->uninstall_plugin != NULL);
  iface->uninstall_plugin (backend, info, cancellable, progress_callback,
                           progress_user_data, callback, user_data);
}

/**
 * peas_gtk_plugin_store_backend_uninstall_plugin_finish:
 * @backend: A #PeasGtkPluginStoreBackend.
 * @result: A #GAsyncResult obtained from the #GAsyncReadyCallback
 *          passed to peas_gtk_plugin_store_backend_uninstall_plugin().
 * @error: Return location for error or %NULL.
 *
 * Finishes an operation started by peas_gtk_plugin_store_backend_uninstall_plugin().
 *
 * Returns: The #PeasGtkInstallablePluginInfo being installed, even if an error occured.
 */
PeasGtkInstallablePluginInfo *
peas_gtk_plugin_store_backend_uninstall_plugin_finish (PeasGtkPluginStoreBackend  *backend,
                                                       GAsyncResult               *result,
                                                       GError                    **error)
{
  PeasGtkPluginStoreBackendInterface *iface;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_STORE_BACKEND (backend), NULL);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  iface = PEAS_GTK_PLUGIN_STORE_BACKEND_GET_IFACE (backend);

  g_assert (iface->uninstall_plugin_finish != NULL);
  return iface->uninstall_plugin_finish (backend, result, error);
}
