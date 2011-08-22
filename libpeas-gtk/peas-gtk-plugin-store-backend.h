/*
 * peas-plugin-store-backend.h
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

#ifndef _PEAS_GTK_PLUGIN_PLUGIN_STORE_BACKEND_H_
#define _PEAS_GTK_PLUGIN_PLUGIN_STORE_BACKEND_H_

#include <gio/gio.h>

#include "peas-gtk-installable-plugin-info.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND            (peas_gtk_plugin_store_backend_get_type ())
#define PEAS_GTK_PLUGIN_STORE_BACKEND(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND, PeasGtkPluginStoreBackend))
#define PEAS_GTK_PLUGIN_STORE_BACKEND_IFACE(obj)      (G_TYPE_CHECK_CLASS_CAST ((obj), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND, PeasGtkPluginStoreBackendInterface))
#define PEAS_GTK_IS_PLUGIN_STORE_BACKEND(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND))
#define PEAS_GTK_PLUGIN_STORE_BACKEND_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND, PeasGtkPluginStoreBackendInterface))

typedef struct _PeasGtkPluginStoreBackend          PeasGtkPluginStoreBackend; /* dummy typedef */
typedef struct _PeasGtkPluginStoreBackendInterface PeasGtkPluginStoreBackendInterface;

/**
 * PeasGtkPluginStoreProgressStatusEnum:
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_STARTING:
 *  The operation is starting.
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_WAITING:
 *  The operation is waiting.
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_WAITING_FOR_AUTH:
 *  The operation is waiting for authentication.
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_DOWNLOADING:
 *  The operation is performing a download.
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_INSTALLING:
 *  The plugin is being installed.
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_UNINSTALLING:
 *  The plugin is being uninstalled.
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_FINISHING:
 *  The operation is finishing.
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_FINISHED:
 *  The operation is finished.
 * @PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_CANCELLED:
 *  The operation has been cancelled.
 */
typedef enum {
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_STARTING,
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_WAITING,
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_WAITING_FOR_AUTH,
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_DOWNLOADING,
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_INSTALLING,
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_UNINSTALLING,
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_FINISHING,
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_FINISHED,
  PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_CANCELLED
} PeasGtkPluginStoreProgressStatusEnum;

/**
 * PeasGtkPluginStoreProgressCallback:
 * @value: The progress that has been made.
 * @message: The progress message.
 * @user_data: Optional data passed to the function.
 *
 * If @value is in the interval [0, 100] then the #PeasGtkPluginStore
 * will display the progress for the operation. If not then it
 * will display activity and in activity mode "pulsing" the callback
 * is unnessesary. If @message is not %NULL then the progress bar's
 * message will be set to @message.
 */
typedef void (*PeasGtkPluginStoreProgressCallback) (gint         value,
                                                    const gchar *message,
                                                    gpointer     user_data);

struct _PeasGtkPluginStoreBackendInterface
{
  GTypeInterface g_iface;

  void        (*get_plugins)              (PeasGtkPluginStoreBackend           *backend,
                                           GCancellable                        *cancellable,
                                           PeasGtkPluginStoreProgressCallback   progress_callback,
                                           gpointer                             progress_user_data,
                                           GAsyncReadyCallback                  callback,
                                           gpointer                             user_data);
  GPtrArray  *(*get_plugins_finish)       (PeasGtkPluginStoreBackend           *backend,
                                           GAsyncResult                        *result,
                                           GError                             **error);

  void        (*install_plugin)           (PeasGtkPluginStoreBackend           *backend,
                                           PeasGtkInstallablePluginInfo        *info,
                                           GCancellable                        *cancellable,
                                           PeasGtkPluginStoreProgressCallback   progress_callback,
                                           gpointer                             progress_user_data,
                                           GAsyncReadyCallback                  callback,
                                           gpointer                             user_data);
  PeasGtkInstallablePluginInfo *
              (*install_plugin_finish)    (PeasGtkPluginStoreBackend           *backend,
                                           GAsyncResult                        *result,
                                           GError                             **error);

  void        (*uninstall_plugin)         (PeasGtkPluginStoreBackend           *backend,
                                           PeasGtkInstallablePluginInfo        *info,
                                           GCancellable                        *cancellable,
                                           PeasGtkPluginStoreProgressCallback   progress_callback,
                                           gpointer                             progress_user_data,
                                           GAsyncReadyCallback                  callback,
                                           gpointer                             user_data);
  PeasGtkInstallablePluginInfo *
              (*uninstall_plugin_finish)  (PeasGtkPluginStoreBackend           *backend,
                                           GAsyncResult                        *result,
                                           GError                             **error);
};

GType peas_gtk_plugin_store_backend_get_type                (void) G_GNUC_CONST;

void  peas_gtk_plugin_store_backend_get_plugins             (PeasGtkPluginStoreBackend           *backend,
                                                             GCancellable                        *cancellable,
                                                             PeasGtkPluginStoreProgressCallback   progress_callback,
                                                             gpointer                             progress_user_data,
                                                             GAsyncReadyCallback                  callback,
                                                             gpointer                             user_data);
GPtrArray *
      peas_gtk_plugin_store_backend_get_plugins_finish      (PeasGtkPluginStoreBackend           *backend,
                                                             GAsyncResult                        *result,
                                                             GError                             **error);

void  peas_gtk_plugin_store_backend_install_plugin          (PeasGtkPluginStoreBackend           *backend,
                                                             PeasGtkInstallablePluginInfo        *info,
                                                             GCancellable                        *cancellable,
                                                             PeasGtkPluginStoreProgressCallback   progress_callback,
                                                             gpointer                             progress_user_data,
                                                             GAsyncReadyCallback                  callback,
                                                             gpointer                             user_data);
PeasGtkInstallablePluginInfo *
      peas_gtk_plugin_store_backend_install_plugin_finish   (PeasGtkPluginStoreBackend           *backend,
                                                             GAsyncResult                        *result,
                                                             GError                             **error);

void  peas_gtk_plugin_store_backend_uninstall_plugin        (PeasGtkPluginStoreBackend           *backend,
                                                             PeasGtkInstallablePluginInfo        *info,
                                                             GCancellable                        *cancellable,
                                                             PeasGtkPluginStoreProgressCallback   progress_callback,
                                                             gpointer                             progress_user_data,
                                                             GAsyncReadyCallback                  callback,
                                                             gpointer                             user_data);
PeasGtkInstallablePluginInfo *
      peas_gtk_plugin_store_backend_uninstall_plugin_finish (PeasGtkPluginStoreBackend           *backend,
                                                             GAsyncResult                        *result,
                                                             GError                             **error);

G_END_DECLS

#endif /* _PEAS_GTK_PLUGIN_PLUGIN_STORE_BACKEND_H_  */
