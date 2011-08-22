/*
 * peas-plugin-store.c
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

#include <string.h>

#include <libpeas/peas-engine.h>
#include <libpeas/peas-plugin-info.h>
#include <libpeas/peas-i18n.h>

#include "peas-gtk-plugin-store.h"
#include "peas-gtk-plugin-store-backend-pk.h"
#include "peas-gtk-plugin-store-model.h"

/**
 * SECTION:peas-gtk-plugin-store
 * @short_description: A UI for (un)installing plugins.
 *
 * The #PeasGtkPluginStore is a widget that can be used to (un)install plugins.
 *
 **/

struct _PeasGtkPluginStorePrivate {
  PeasEngine *engine;
  PeasGtkPluginStoreBackend *backend;
  GCancellable *get_plugins_cancellable;

  GtkWidget *info_bar_holder;
  GtkInfoBar *info_bar;

  GtkTreeView *view;
  PeasGtkPluginStoreModel *model;
};

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE,
  N_PROPERTIES
};

/* Signals */
enum {
  BACK,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];
static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE (PeasGtkPluginStore, peas_gtk_plugin_store, GTK_TYPE_BOX)

typedef struct {
  PeasGtkPluginStore *store;

  GtkProgressBar *progress_bar;
  guint pulse_id;
} GetPluginsAsyncData;

typedef struct {
  PeasGtkPluginStore *store;
  PeasGtkInstallablePluginInfo *info;
  guint pulse_id;
} AsyncData;

enum {
  RESPONSE_TRY_AGAIN = 1
};

static void
info_bar_response_cb (GtkInfoBar         *info_bar,
                      gint                response_id,
                      PeasGtkPluginStore *store)
{
  switch (response_id)
    {
    case GTK_RESPONSE_CLOSE:
      gtk_widget_destroy (GTK_WIDGET (info_bar));
      store->priv->info_bar = NULL;
      /* Otherwise we get spacing for the box */
      gtk_widget_hide (store->priv->info_bar_holder);
      break;
    /*case RESPONSE_TRY_AGAIN:
      ...*/
    default:
      break;
    }
}

static const gchar *
message_type_to_stock (GtkMessageType message_type)
{
  switch (message_type)
    {
    case GTK_MESSAGE_INFO:
      return GTK_STOCK_DIALOG_INFO;
    case GTK_MESSAGE_WARNING:
      return GTK_STOCK_DIALOG_WARNING;
    case GTK_MESSAGE_QUESTION:
      return GTK_STOCK_DIALOG_QUESTION;
    case GTK_MESSAGE_ERROR:
      return GTK_STOCK_DIALOG_ERROR;
    default:
      return NULL;
    }
}

static GtkWidget *
setup_info_bar (PeasGtkPluginStore *store,
                GtkMessageType      message_type,
                const gchar        *primary_text,
                const gchar        *secondary_text)
{
  GtkWidget *content_area;
  GtkWidget *hbox_area;
  const gchar *stock_id;
  GtkWidget *image;
  GtkWidget *vbox_area;
  gchar *primary_markup;
  GtkWidget *primary_label;

  if (store->priv->info_bar != NULL)
    gtk_widget_destroy (GTK_WIDGET (store->priv->info_bar));

  store->priv->info_bar = GTK_INFO_BAR (gtk_info_bar_new ());
  gtk_info_bar_set_message_type (store->priv->info_bar, message_type);
  gtk_box_pack_start (GTK_BOX (store->priv->info_bar_holder),
                      GTK_WIDGET (store->priv->info_bar), TRUE, TRUE, 0);

  content_area = gtk_info_bar_get_content_area (store->priv->info_bar);

	hbox_area = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_box_pack_start (GTK_BOX (content_area), hbox_area, TRUE, TRUE, 0);

  stock_id = message_type_to_stock (message_type);

  if (stock_id != NULL)
    {
	    image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_DIALOG);
	    gtk_widget_set_valign (image, GTK_ALIGN_START);
	    gtk_box_pack_start (GTK_BOX (hbox_area), image, FALSE, FALSE, 0);
    }

  vbox_area = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (hbox_area), vbox_area, TRUE, TRUE, 0);

  primary_markup = g_markup_printf_escaped ("<b>%s</b>", primary_text);
  primary_label = gtk_label_new (primary_markup);
  gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
  gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
  gtk_widget_set_halign (primary_label, GTK_ALIGN_START);
  gtk_widget_set_can_focus (primary_label, TRUE);
  gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_area), primary_label, TRUE, TRUE, 0);

  if (secondary_text != NULL)
    {
      gchar *secondary_markup;
      GtkWidget *secondary_label;

      secondary_markup = g_markup_printf_escaped ("<small>%s</small>",
                                                  secondary_text);
      secondary_label = gtk_label_new (secondary_markup);
      gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
      gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
      gtk_widget_set_halign (secondary_label, GTK_ALIGN_START);
      gtk_widget_set_can_focus (secondary_label, TRUE);
      gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
      gtk_box_pack_start (GTK_BOX (vbox_area), secondary_label, TRUE, TRUE, 0);

      g_free (secondary_markup);
    }

  g_signal_connect (store->priv->info_bar,
                    "response",
                    G_CALLBACK (info_bar_response_cb),
                    store);

  gtk_widget_show_all (store->priv->info_bar_holder);

  g_free (primary_markup);

  return vbox_area;
}

static GetPluginsAsyncData *
get_plugins_async_data_new (PeasGtkPluginStore *store)
{
  GetPluginsAsyncData *data;

  data = g_new (GetPluginsAsyncData, 1);
  data->store = g_object_ref (store);
  data->pulse_id = 0;

  if (store->priv->get_plugins_cancellable != NULL)
    {
      g_cancellable_cancel (store->priv->get_plugins_cancellable);
      g_object_unref (store->priv->get_plugins_cancellable);
    }

  store->priv->get_plugins_cancellable = g_cancellable_new ();

  return data;
}

static void
get_plugins_async_data_free (GetPluginsAsyncData *data)
{
  if (data->pulse_id != 0)
    g_source_remove (data->pulse_id);

  g_object_unref (data->store);
  g_free (data);
}

static void
get_plugins_progress_cb (gint                 value,
                         const gchar         *message,
                         GetPluginsAsyncData *data)
{
  if (message != NULL)
    gtk_progress_bar_set_text (data->progress_bar, message);

  if (value >= 0 && value <= 100)
    {
      if (data->pulse_id != 0)
        {
          g_source_remove (data->pulse_id);
          data->pulse_id = 0;
        }

      gtk_progress_bar_set_fraction (data->progress_bar, value / 100.0);
    }
  else if (data->pulse_id == 0)
    {
      data->pulse_id = g_timeout_add (1000 / 10,
                                      (GSourceFunc) gtk_progress_bar_pulse,
                                      data->progress_bar);
    }

  gtk_widget_show (GTK_WIDGET (data->progress_bar));
}

static AsyncData *
async_data_new (PeasGtkPluginStore           *store,
                PeasGtkInstallablePluginInfo *info)
{
  GtkTreeIter iter;
  AsyncData *data;

  data = g_new (AsyncData, 1);
  data->store = g_object_ref (store);
  data->info = info;
  data->pulse_id = 0;

  peas_gtk_plugin_store_model_get_iter_from_plugin (store->priv->model,
                                                    &iter, info);

  gtk_list_store_set (GTK_LIST_STORE (store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_COLUMN, 0,
                      -1);

  return data;
}

static void
async_data_free (AsyncData *data)
{
  GtkTreeIter iter;

  peas_gtk_plugin_store_model_get_iter_from_plugin (data->store->priv->model,
                                                    &iter, data->info);

  if (data->pulse_id != 0)
    g_source_remove (data->pulse_id);

  g_object_unref (data->store);
  g_free (data);
}

static gboolean
un_install_plugin_pulse_progress_cb (AsyncData *data)
{
  GtkTreeIter iter;
  gint pulse;

  peas_gtk_plugin_store_model_get_iter_from_plugin (data->store->priv->model,
                                                    &iter, data->info);

  gtk_tree_model_get (GTK_TREE_MODEL (data->store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN, &pulse,
                      -1);

  /* Avoid G_MAXINT as it would show us as completed and
   * reset pulse to 1 showing that we have started.
   */
  if (++pulse == G_MAXINT)
    pulse = 1;

  gtk_list_store_set (GTK_LIST_STORE (data->store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN, pulse,
                      -1);

  return TRUE;
}

/*
static gboolean
un_install_plugin_update_status (AsyncData *data)
{
  const gchar *message;
  GtkTreeIter iter;

  switch (data->status)
    {
    case PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_FINISHED:
      message = _("Finished");
      break;
    ...
    default:
      g_return_val_if_reached (FALSE);
      break;
    }

  peas_gtk_plugin_store_model_get_iter_from_plugin (data->store->priv->model,
                                                    &iter, data->info);

  gtk_list_store_set (GTK_LIST_STORE (data->store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_MESSAGE_COLUMN, message,
                      -1);

  data->status_id = 0;
  return FALSE;
}
*/

static void
un_install_plugin_progress_cb (gint         value,
                               const gchar *message,
                               AsyncData   *data)
{
  GtkTreeIter iter;

  peas_gtk_plugin_store_model_get_iter_from_plugin (data->store->priv->model,
                                                    &iter, data->info);

  if (message != NULL)
    {
      gtk_list_store_set (GTK_LIST_STORE (data->store->priv->model), &iter,
                          PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_MESSAGE_COLUMN, message,
                          -1);

#if 0
      /* Delay showing a new message so avoid spurious updates */
      data->status = status;

      /* Show immediately */
      if (status == PEAS_GTK_PLUGIN_STORE_PROGRESS_STATUS_FINISHED)
        {
          un_install_plugin_set_status (data);
        }
      else if (data->status_id == 0)
        {
          data->status_id = g_timeout_add_seconds (2,
              (GSourceFunc) un_install_plugin_set_status, data);
        }
#endif
    }

  if (value >= 0 && value <= 100)
    {
      if (data->pulse_id != 0)
        {
          g_source_remove (data->pulse_id);
          data->pulse_id = 0;
        }

      /* Must set pulse to -1 to make sure we show the percentage */
      gtk_list_store_set (GTK_LIST_STORE (data->store->priv->model), &iter,
                          PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN, -1,
                          PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_COLUMN, value,
                          -1);
    }
  else if (data->pulse_id == 0)
    {
      data->pulse_id = g_timeout_add (1000 / 10,
                                      (GSourceFunc) un_install_plugin_pulse_progress_cb,
                                      data);
    }
}

static void
get_plugins_cb (PeasGtkPluginStoreBackend *backend,
                GAsyncResult              *result,
                GetPluginsAsyncData       *data)
{
  PeasGtkPluginStore *store = data->store;
  GPtrArray *plugins;
  GError *error = NULL;

  plugins = peas_gtk_plugin_store_backend_get_plugins_finish (backend, result,
                                                              &error);

  gtk_info_bar_response (store->priv->info_bar, GTK_RESPONSE_CLOSE);

  if (error != NULL)
    {
      if (error->code != G_IO_ERROR_CANCELLED)
        {
          setup_info_bar (store, GTK_MESSAGE_ERROR,
                          _("Failed to get plugins."),
                          error->message);
          /*gtk_info_bar_add_button (store->priv->info_bar,
                                   _("_Try Again"), RESPONSE_TRY_AGAIN);*/
        }

      g_error_free (error);
      goto out;
    }

  peas_gtk_plugin_store_model_reload (store->priv->model, plugins);
  gtk_tree_view_set_model (store->priv->view,
                           GTK_TREE_MODEL (store->priv->model));

  g_ptr_array_unref (plugins);

out:

  get_plugins_async_data_free (data);
}

static void
install_plugin_cb (PeasGtkPluginStoreBackend *backend,
                   GAsyncResult              *result,
                   AsyncData                 *data)
{
  PeasGtkPluginStore *store = data->store;
  PeasGtkInstallablePluginInfo *info;
  GError *error = NULL;

  info = peas_gtk_plugin_store_backend_install_plugin_finish (backend, result,
                                                              &error);

  peas_gtk_plugin_store_model_reload_plugin (store->priv->model, info);

  if (error != NULL)
    {
      gchar *text = g_strdup_printf (_("Failed to install plugin \"%s\"."),
                                     peas_gtk_installable_plugin_info_get_name (info));

      setup_info_bar (store, GTK_MESSAGE_ERROR, text, error->message);
      gtk_info_bar_add_button (store->priv->info_bar,
                               GTK_STOCK_OK, GTK_RESPONSE_CLOSE);

      g_free (text);
      g_error_free (error);
    }

  async_data_free (data);
}

static void
uninstall_plugin_cb (PeasGtkPluginStoreBackend *backend,
                     GAsyncResult              *result,
                     AsyncData                 *data)
{
  PeasGtkPluginStore *store = data->store;
  PeasGtkInstallablePluginInfo *info;
  GError *error = NULL;

  info = peas_gtk_plugin_store_backend_uninstall_plugin_finish (backend,
                                                                result,
                                                                &error);

  peas_gtk_plugin_store_model_reload_plugin (store->priv->model, info);

  if (error != NULL)
    {
      gchar *text = g_strdup_printf (_("Failed to uninstall plugin \"%s\"."),
                                     peas_gtk_installable_plugin_info_get_name (info));

      setup_info_bar (store, GTK_MESSAGE_ERROR, text, error->message);
      gtk_info_bar_add_button (store->priv->info_bar,
                               GTK_STOCK_OK, GTK_RESPONSE_CLOSE);

      g_free (text);
    }

  async_data_free (data);
}

static void
get_plugins (PeasGtkPluginStore *store)
{
  GetPluginsAsyncData *data;
  GtkWidget *content_area;

  data = get_plugins_async_data_new (store);

  content_area = setup_info_bar (store, GTK_MESSAGE_OTHER,
                                 _("Loading plugins"), NULL);

  /* We do not show the progress bar here so backends
   * can choose whether a progress bar would be informational.
   */
  data->progress_bar = GTK_PROGRESS_BAR (gtk_progress_bar_new ());
  gtk_progress_bar_set_show_text (data->progress_bar, TRUE);
  gtk_box_pack_start (GTK_BOX (content_area), GTK_WIDGET (data->progress_bar),
                      TRUE, TRUE, 0);

  peas_gtk_plugin_store_backend_get_plugins (store->priv->backend,
                                             store->priv->get_plugins_cancellable,
                                             (PeasGtkPluginStoreProgressCallback) get_plugins_progress_cb,
                                             data,
                                             (GAsyncReadyCallback) get_plugins_cb,
                                             data);
}

static gboolean
view_query_tooltip_cb (GtkWidget          *widget,
                       gint                x,
                       gint                y,
                       gboolean            keyboard_mode,
                       GtkTooltip         *tooltip,
                       PeasGtkPluginStore *store)
{
  gboolean is_row;
  GtkTreeIter iter;
  PeasGtkInstallablePluginInfo *info;
  gchar *message;
  GError *error = NULL;

  is_row = gtk_tree_view_get_tooltip_context (store->priv->view,
                                              &x, &y, keyboard_mode,
                                              NULL, NULL, &iter);

  if (!is_row)
    return FALSE;

  info = peas_gtk_plugin_store_model_get_plugin (store->priv->model, &iter);

  if (peas_gtk_installable_plugin_info_is_available (info, &error))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (store->priv->model), &iter,
        PEAS_GTK_PLUGIN_STORE_MODEL_INFO_COLUMN, &message,
        -1);
    }
  else
    {
      const gchar *name;

      name = peas_gtk_installable_plugin_info_get_name (info);
      message = g_markup_printf_escaped (_("<b>%s</b>\nAn error occurred: %s"),
                                         name, error->message);

      g_error_free (error);
    }

  gtk_tooltip_set_markup (tooltip, message);

  g_free (message);

  return TRUE;
}

static void
installed_toggled_cb (GtkCellRendererToggle *cell,
                      gchar                 *path_str,
                      PeasGtkPluginStore    *store)
{
  GtkTreeModel *tree_model = GTK_TREE_MODEL (store->priv->model);
  GtkTreePath *path;
  GtkTreeIter iter;

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_model, &iter, path))
    {
      PeasGtkInstallablePluginInfo *info;

      info = peas_gtk_plugin_store_model_get_plugin (store->priv->model,
                                                     &iter);

      if (!peas_gtk_installable_plugin_info_is_installed (info))
        peas_gtk_plugin_store_install_plugin (store, info);
      else
        peas_gtk_plugin_store_uninstall_plugin (store, info);
    }

  gtk_tree_path_free (path);
}

/* Callback used as the interactive search comparison function */
static gboolean
name_search_cb (GtkTreeModel       *model,
                gint                column,
                const gchar        *key,
                GtkTreeIter        *iter,
                PeasGtkPluginStore *store)
{
  PeasGtkInstallablePluginInfo *info;
  gchar *normalized_string;
  gchar *normalized_key;
  gchar *case_normalized_string;
  gchar *case_normalized_key;
  gint key_len;
  gboolean retval;

  info = peas_gtk_plugin_store_model_get_plugin (store->priv->model, iter);

  if (info == NULL)
    return FALSE;

  normalized_string = g_utf8_normalize (peas_gtk_installable_plugin_info_get_name (info), -1, G_NORMALIZE_ALL);
  normalized_key = g_utf8_normalize (key, -1, G_NORMALIZE_ALL);
  case_normalized_string = g_utf8_casefold (normalized_string, -1);
  case_normalized_key = g_utf8_casefold (normalized_key, -1);

  key_len = strlen (case_normalized_key);

  /* Oddly enough, this callback must return whether to stop the search
   * because we found a match, not whether we actually matched.
   */
  retval = strncmp (case_normalized_key, case_normalized_string, key_len) != 0;

  g_free (normalized_key);
  g_free (normalized_string);
  g_free (case_normalized_key);
  g_free (case_normalized_string);

  return retval;
}

static void
plugin_icon_data_func (GtkTreeViewColumn *column,
                       GtkCellRenderer   *cell,
                       GtkTreeModel      *model,
                       GtkTreeIter       *iter)
{
  GdkPixbuf *icon_pixbuf;
  gchar *icon_name;

  gtk_tree_model_get (model, iter,
    PEAS_GTK_PLUGIN_STORE_MODEL_ICON_PIXBUF_COLUMN, &icon_pixbuf,
    PEAS_GTK_PLUGIN_STORE_MODEL_ICON_NAME_COLUMN,   &icon_name,
    -1);

  if (icon_pixbuf == NULL)
    {
      g_object_set (cell, "icon-name", icon_name, NULL);
    }
  else
    {
      g_object_set (cell, "pixbuf", icon_pixbuf, NULL);
      g_object_unref (icon_pixbuf);
    }

  g_free (icon_name);
}

static void
back_button_clicked_cb (GtkWidget          *widget,
                        PeasGtkPluginStore *store)
{
  g_signal_emit (store, signals[BACK], 0);
}

static void
peas_gtk_plugin_store_init (PeasGtkPluginStore *store)
{
  GtkWidget *sw;
  GtkStyleContext *context;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  GtkCellArea *area;
  GtkWidget *toolbar;
  GtkToolItem *toolitem;
  GtkWidget *back_button;

  store->priv = G_TYPE_INSTANCE_GET_PRIVATE (store,
                                             PEAS_GTK_TYPE_PLUGIN_STORE,
                                             PeasGtkPluginStorePrivate);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (store),
                                  GTK_ORIENTATION_VERTICAL);

  store->priv->info_bar_holder = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (store), store->priv->info_bar_holder,
                      FALSE, TRUE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
  context = gtk_widget_get_style_context (sw);
  gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
  gtk_box_pack_start (GTK_BOX (store), sw, TRUE, TRUE, 0);

  store->priv->view = GTK_TREE_VIEW (gtk_tree_view_new ());
  gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (store->priv->view));

  gtk_widget_set_has_tooltip (GTK_WIDGET (store->priv->view), TRUE);
  g_signal_connect (store->priv->view,
                    "query-tooltip",
                    G_CALLBACK (view_query_tooltip_cb),
                    store);

  gtk_tree_view_set_rules_hint (store->priv->view, TRUE);
  gtk_tree_view_set_headers_visible (store->priv->view, FALSE);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_spacing (column, 6);
  gtk_tree_view_append_column (store->priv->view, column);

  cell = gtk_cell_renderer_toggle_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  g_object_set (cell, "xpad", 6, NULL);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "active", PEAS_GTK_PLUGIN_STORE_MODEL_INSTALLED_COLUMN,
                                       "activatable", PEAS_GTK_PLUGIN_STORE_MODEL_NOT_IN_USE_COLUMN,
                                       "sensitive", PEAS_GTK_PLUGIN_STORE_MODEL_NOT_IN_USE_COLUMN,
                                       "visible", PEAS_GTK_PLUGIN_STORE_MODEL_AVAILABLE_COLUMN,
                                       NULL);
  g_signal_connect (cell,
                    "toggled",
                    G_CALLBACK (installed_toggled_cb),
                    store);

  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  g_object_set (cell, "stock-size", GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
  gtk_tree_view_column_set_cell_data_func (column, cell,
                                           (GtkTreeCellDataFunc) plugin_icon_data_func,
                                           NULL, NULL);


  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_spacing (column, 6);
  area = gtk_cell_layout_get_area (GTK_CELL_LAYOUT (column));
  gtk_orientable_set_orientation (GTK_ORIENTABLE (area), GTK_ORIENTATION_VERTICAL);
  gtk_tree_view_append_column (store->priv->view, column);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, TRUE);
  g_object_set (cell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "markup", PEAS_GTK_PLUGIN_STORE_MODEL_INFO_COLUMN,
                                       NULL);

  cell = gtk_cell_renderer_progress_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "visible", PEAS_GTK_PLUGIN_STORE_MODEL_ACTIVE_COLUMN,
                                       "pulse", PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN,
                                       "value", PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_COLUMN,
                                       "text", PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_MESSAGE_COLUMN,
                                       NULL);

  /* Enable search for our non-string column */
  gtk_tree_view_set_search_column (store->priv->view,
                                   PEAS_GTK_PLUGIN_STORE_MODEL_PLUGIN_COLUMN);
  gtk_tree_view_set_search_equal_func (store->priv->view,
                                       (GtkTreeViewSearchEqualFunc) name_search_cb,
                                       store,
                                       NULL);

  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_MENU);
  context = gtk_widget_get_style_context (toolbar);
  gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_INLINE_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (store), toolbar, FALSE, FALSE, 0);

  toolitem = gtk_tool_item_new ();
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, -1);

  back_button = gtk_button_new_with_mnemonic (_("_Plugin List"));
  gtk_button_set_image (GTK_BUTTON (back_button),
                        gtk_image_new_from_stock (GTK_STOCK_GO_BACK,
                                                  GTK_ICON_SIZE_BUTTON));
  gtk_container_add (GTK_CONTAINER (toolitem), back_button);

  g_signal_connect (back_button,
                    "clicked",
                    G_CALLBACK (back_button_clicked_cb),
                    store);

  gtk_widget_set_size_request (GTK_WIDGET (store->priv->view), 370, 200);

  gtk_widget_show_all (GTK_WIDGET (store));
  gtk_widget_hide (GTK_WIDGET (store));
  gtk_widget_hide (store->priv->info_bar_holder);
}

static void
peas_gtk_plugin_store_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *storepec)
{
  PeasGtkPluginStore *store = PEAS_GTK_PLUGIN_STORE (object);

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
peas_gtk_plugin_store_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *storepec)
{
  PeasGtkPluginStore *store = PEAS_GTK_PLUGIN_STORE (object);

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
peas_gtk_plugin_store_constructed (GObject *object)
{
  PeasGtkPluginStore *store = PEAS_GTK_PLUGIN_STORE (object);

  if (store->priv->engine == NULL)
    store->priv->engine = peas_engine_get_default ();

  g_object_ref (store->priv->engine);

  store->priv->backend = peas_gtk_plugin_store_backend_pk_new (store->priv->engine);
  store->priv->model = peas_gtk_plugin_store_model_new (store->priv->engine);

  get_plugins (store);

  G_OBJECT_CLASS (peas_gtk_plugin_store_parent_class)->constructed (object);
}

static void
peas_gtk_plugin_store_dispose (GObject *object)
{
  PeasGtkPluginStore *store = PEAS_GTK_PLUGIN_STORE (object);

  if (store->priv->get_plugins_cancellable != NULL)
    {
      g_cancellable_cancel (store->priv->get_plugins_cancellable);
      g_clear_object (&store->priv->get_plugins_cancellable);
    }

  g_clear_object (&store->priv->engine);
  g_clear_object (&store->priv->backend);
  g_clear_object (&store->priv->model);

  G_OBJECT_CLASS (peas_gtk_plugin_store_parent_class)->dispose (object);
}

static void
peas_gtk_plugin_store_class_init (PeasGtkPluginStoreClass *klass)
{
  GType the_type = G_TYPE_FROM_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_gtk_plugin_store_set_property;
  object_class->get_property = peas_gtk_plugin_store_get_property;
  object_class->constructed = peas_gtk_plugin_store_constructed;
  object_class->dispose = peas_gtk_plugin_store_dispose;

  /**
   * PeasGtkPluginStore:engine:
   *
   * The #PeasEngine this store is attached to.
   */
  properties[PROP_ENGINE] =
    g_param_spec_object ("engine",
                         "engine",
                         "The PeasEngine this store is attached to",
                         PEAS_TYPE_ENGINE,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * PeasGtkPluginStore::back:
   * @store: A #PeasGtkPluginStore.
   *
   * The back signal is emitted when the manager
   * should show the plugin list again.
   */
  signals[BACK] =
    g_signal_new ("back",
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasGtkPluginStoreClass, back),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
  g_type_class_add_private (object_class, sizeof (PeasGtkPluginStorePrivate));
}

/**
 * peas_gtk_plugin_store_new:
 * @engine: (allow-none): A #PeasEngine, or %NULL.
 *
 * Creates a new plugin store for the given #PeasEngine.
 *
 * If @engine is %NULL, then the default engine will be used.
 *
 * Returns: the new #PeasGtkPluginStore.
 */
GtkWidget *
peas_gtk_plugin_store_new (PeasEngine *engine)
{
  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);

  return GTK_WIDGET (g_object_new (PEAS_GTK_TYPE_PLUGIN_STORE,
                                   "engine", engine,
                                   NULL));
}

/**
 * peas_gtk_plugin_store_set_selected_plugin:
 * @store: A #PeasGtkPluginStore.
 * @info: A #PeasGtkInstallablePluginInfo.
 *
 * Selects the given plugin.
 */
void
peas_gtk_plugin_store_set_selected_plugin (PeasGtkPluginStore           *store,
                                           PeasGtkInstallablePluginInfo *info)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE (store));
  g_return_if_fail (info != NULL);
  g_return_if_fail (peas_gtk_plugin_store_model_get_iter_from_plugin (store->priv->model,
                                                                      &iter, info));

  selection = gtk_tree_view_get_selection (store->priv->view);
  gtk_tree_selection_select_iter (selection, &iter);
}

/**
 * peas_gtk_plugin_store_get_selected_plugin:
 * @store: A #PeasGtkPluginStore.
 *
 * Returns the currently selected plugin, or %NULL if a plugin is not selected.
 *
 * Returns: the selected plugin.
 */
PeasGtkInstallablePluginInfo *
peas_gtk_plugin_store_get_selected_plugin (PeasGtkPluginStore *store)
{
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  PeasGtkInstallablePluginInfo *info = NULL;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_STORE (store), NULL);

  selection = gtk_tree_view_get_selection (store->priv->view);

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    info = peas_gtk_plugin_store_model_get_plugin (store->priv->model, &iter);

  return info;
}

/**
 * peas_gtk_plugin_store_install_plugin:
 * @store: A #PeasGtkPluginStore.
 * @info: A #PeasGtkInstallablePluginInfo.
 *
 * Installs @info.
 */
void
peas_gtk_plugin_store_install_plugin (PeasGtkPluginStore           *store,
                                      PeasGtkInstallablePluginInfo *info)
{
  AsyncData *data;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE (store));
  g_return_if_fail (info != NULL);
  g_return_if_fail (peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_installed (info));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_in_use (info));

  data = async_data_new (store, info);

  peas_gtk_plugin_store_backend_install_plugin (store->priv->backend,
      info, NULL,
      (PeasGtkPluginStoreProgressCallback) un_install_plugin_progress_cb, data,
      (GAsyncReadyCallback) install_plugin_cb, data);

  peas_gtk_plugin_store_model_reload_plugin (store->priv->model, info);
}

/**
 * peas_gtk_plugin_store_uninstall_plugin:
 * @store: A #PeasGtkPluginStore.
 * @info: A #PeasGtkInstallablePluginInfo.
 *
 * Uninstalls @info.
 */
void
peas_gtk_plugin_store_uninstall_plugin (PeasGtkPluginStore           *store,
                                        PeasGtkInstallablePluginInfo *info)
{
  AsyncData *data;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE (store));
  g_return_if_fail (info != NULL);
  g_return_if_fail (peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (peas_gtk_installable_plugin_info_is_installed (info));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_in_use (info));

  data = async_data_new (store, info);

  peas_gtk_plugin_store_backend_uninstall_plugin (store->priv->backend,
      info, NULL,
      (PeasGtkPluginStoreProgressCallback) un_install_plugin_progress_cb, data,
      (GAsyncReadyCallback) uninstall_plugin_cb, data);

  peas_gtk_plugin_store_model_reload_plugin (store->priv->model, info);
}
