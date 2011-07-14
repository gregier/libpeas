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
#include "peas-gtk-plugin-store-backend-xml.h"
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
  PeasGtkPluginStoreModel *model;

  GCancellable *get_plugins_cancellable;

  GtkWidget *popup_menu;
};

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE
};

/* Signals */
enum {
  POPULATE_POPUP,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE (PeasGtkPluginStore, peas_gtk_plugin_store, GTK_TYPE_TREE_VIEW)

typedef struct {
  PeasGtkPluginStore *store;
  PeasGtkInstallablePluginInfo *info;
  guint pulse_id;
} ProgressData;

static ProgressData *
start_progress (PeasGtkPluginStore           *store,
                PeasGtkInstallablePluginInfo *info)
{
  GtkTreeIter iter;
  ProgressData *progress;

  progress = g_new (ProgressData, 1);
  progress->store = store;
  progress->info = info;
  progress->pulse_id = 0;

  peas_gtk_plugin_store_model_get_iter_from_plugin (store->priv->model,
                                                    &iter, info);

  gtk_list_store_set (GTK_LIST_STORE (store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_ACTIVE_COLUMN, TRUE,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_COLUMN, 0,
                      -1);

  return progress;
}

static void
stop_progress (PeasGtkPluginStore           *store,
               PeasGtkInstallablePluginInfo *info)
{
  GtkTreeIter iter;

  peas_gtk_plugin_store_model_get_iter_from_plugin (store->priv->model,
                                                    &iter, info);

  gtk_list_store_set (GTK_LIST_STORE (store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_ACTIVE_COLUMN, FALSE,
                      -1);
}

static void
set_progress (PeasGtkPluginStore           *store,
              PeasGtkInstallablePluginInfo *info,
              gint                          progress)
{
  GtkTreeIter iter;

  peas_gtk_plugin_store_model_get_iter_from_plugin (store->priv->model,
                                                    &iter, info);

  /* Must set pulse to -1 to make sure we show the percentage */
  gtk_list_store_set (GTK_LIST_STORE (store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN, -1,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_COLUMN, progress,
                      -1);
}

static gboolean
pulse_progress_cb (ProgressData *progress)
{
  GtkTreeIter iter;
  gint pulse;
  gboolean active;

  peas_gtk_plugin_store_model_get_iter_from_plugin (progress->store->priv->model,
                                                    &iter, progress->info);

  gtk_tree_model_get (GTK_TREE_MODEL (progress->store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN, &pulse,
                      PEAS_GTK_PLUGIN_STORE_MODEL_ACTIVE_COLUMN, &active,
                      -1);

  if (!active)
    {
      g_free (progress);
      return FALSE;
    }

  /* Avoid G_MAXINT as it would show us as completed and
   * reset pulse to 1 showing that we have started
   */ 
  if (++pulse == G_MAXINT)
    pulse = 1;

  gtk_list_store_set (GTK_LIST_STORE (progress->store->priv->model), &iter,
                      PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN, pulse,
                      -1);

  return TRUE;
}

static void
plugin_progress_cb (gint          value,
                    ProgressData *progress)
{
  if (value >= 0 && value <= 100)
    {
      if (progress->pulse_id != 0)
        {
          g_source_remove (progress->pulse_id);
          progress->pulse_id = 0;
        }

      set_progress (progress->store, progress->info, value);
    }
  else if (progress->pulse_id == 0)
    {
      progress->pulse_id = g_timeout_add (1000 / 20,
                                          (GSourceFunc) pulse_progress_cb,
                                          progress);
    }
}

static void
get_plugins_cb (PeasGtkPluginStoreBackend *backend,
                GAsyncResult              *result,
                PeasGtkPluginStore        *store)
{
  GPtrArray *plugins;
  GError *error = NULL;

  plugins = peas_gtk_plugin_store_backend_get_plugins_finish (backend, result,
                                                              &error);

  if (error != NULL)
    {
      if (error->code != G_IO_ERROR_CANCELLED)
        g_warning ("%s", error->message);

      g_error_free (error);
      goto out;
    }

  peas_gtk_plugin_store_model_reload (store->priv->model, plugins);

  store->priv->get_plugins_cancellable = NULL;
  g_ptr_array_unref (plugins);

out:

  g_object_unref (store);
}

static void
install_plugin_cb (PeasGtkPluginStoreBackend *backend,
                   GAsyncResult              *result,
                   PeasGtkPluginStore        *store)
{
  PeasGtkInstallablePluginInfo *info;
  GError *error = NULL;

  info = peas_gtk_plugin_store_backend_install_plugin_finish (backend, result,
                                                              &error);

  stop_progress (store, info);
  peas_gtk_plugin_store_model_reload_plugin (store->priv->model, info);

  if (error != NULL)
    {
      if (error->code != G_IO_ERROR_CANCELLED)
        g_warning ("%s", error->message);

      g_error_free (error);
      goto out;
    }

out:

  g_object_unref (store);
}

static void
uninstall_plugin_cb (PeasGtkPluginStoreBackend *backend,
                     GAsyncResult              *result,
                     PeasGtkPluginStore        *store)
{
  PeasGtkInstallablePluginInfo *info;
  GError *error = NULL;

  info = peas_gtk_plugin_store_backend_uninstall_plugin_finish (backend,
                                                                result,
                                                                &error);

  stop_progress (store, info);
  peas_gtk_plugin_store_model_reload_plugin (store->priv->model, info);

  if (error != NULL)
    {
      if (error->code != G_IO_ERROR_CANCELLED)
        g_warning ("%s", error->message);

      g_error_free (error);
      goto out;
    }

out:

  g_object_unref (store);
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

static GtkWidget *
create_popup_menu (PeasGtkPluginStore *store)
{
  PeasGtkInstallablePluginInfo *info;
  GtkWidget *menu;
  /*GtkWidget *item;*/

  info = peas_gtk_plugin_store_get_selected_plugin (store);

  if (info == NULL)
    return NULL;

  menu = gtk_menu_new ();

#if 0
  item = gtk_check_menu_item_new_with_mnemonic (_("_Enabled"));
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                  peas_gtk_installable_plugin_info_is_loaded (info));
  g_signal_connect (item, "toggled", G_CALLBACK (enabled_menu_cb), list);
  gtk_widget_set_sensitive (item, peas_gtk_installable_plugin_info_is_available (info, NULL) &&
                                  !peas_gtk_installable_plugin_info_is_builtin (info));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_menu_item_new_with_mnemonic (_("E_nable All"));
  g_signal_connect (item, "activate", G_CALLBACK (enable_all_menu_cb), list);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_menu_item_new_with_mnemonic (_("_Disable All"));
  g_signal_connect (item, "activate", G_CALLBACK (disable_all_menu_cb), list);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
#endif

  g_signal_emit (store, signals[POPULATE_POPUP], 0, menu);

  gtk_widget_show_all (menu);

  return menu;
}

static void
popup_menu_detach (PeasGtkPluginStore *store,
                   GtkMenu            *menu)
{
  store->priv->popup_menu = NULL;
}

static void
menu_position_under_view (GtkMenu     *menu,
                          gint        *x,
                          gint        *y,
                          gboolean    *push_in,
                          GtkTreeView *view)
{
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GdkWindow *window;

  selection = gtk_tree_view_get_selection (view);

  window = gtk_widget_get_window (GTK_WIDGET (view));
  gdk_window_get_origin (window, x, y);

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      GtkTreeModel *model;
      GtkTreePath *path;
      GdkRectangle rect;

      model = gtk_tree_view_get_model (view);
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_view_get_cell_area (view,
                                   path,
                                   gtk_tree_view_get_column (view, 0), /* FIXME 0 for RTL ? */
                                   &rect);
      gtk_tree_path_free (path);

      *x += rect.x;
      *y += rect.y + rect.height;

      if (gtk_widget_get_direction (GTK_WIDGET (view)) == GTK_TEXT_DIR_RTL)
        {
          GtkRequisition requisition;
          gtk_widget_get_preferred_size (GTK_WIDGET (menu), &requisition,
                                         NULL);
          *x += rect.width - requisition.width;
        }
    }
  else
    {
      GtkAllocation allocation;

      gtk_widget_get_allocation (GTK_WIDGET (view), &allocation);

      *x += allocation.x;
      *y += allocation.y;

      if (gtk_widget_get_direction (GTK_WIDGET (view)) == GTK_TEXT_DIR_RTL)
        {
          GtkRequisition requisition;

          gtk_widget_get_preferred_size (GTK_WIDGET (menu), &requisition,
                                         NULL);

          *x += allocation.width - requisition.width;
        }
    }

  *push_in = TRUE;
}

static gboolean
show_popup_menu (GtkTreeView        *view,
                 PeasGtkPluginStore *store,
                 GdkEventButton     *event)
{
  if (store->priv->popup_menu)
    gtk_widget_destroy (store->priv->popup_menu);

  store->priv->popup_menu = create_popup_menu (store);

  if (store->priv->popup_menu == NULL)
    return FALSE;

  gtk_menu_attach_to_widget (GTK_MENU (store->priv->popup_menu),
                             GTK_WIDGET (store),
                             (GtkMenuDetachFunc) popup_menu_detach);

  if (event != NULL)
    {
      gtk_menu_popup (GTK_MENU (store->priv->popup_menu), NULL, NULL,
                      NULL, NULL, event->button, event->time);
    }
  else
    {
      gtk_menu_popup (GTK_MENU (store->priv->popup_menu), NULL, NULL,
                      (GtkMenuPositionFunc) menu_position_under_view,
                      store, 0, gtk_get_current_event_time ());

      gtk_menu_shell_select_first (GTK_MENU_SHELL (store->priv->popup_menu),
                                   FALSE);
    }

  return TRUE;
}

static void
plugin_icon_data_func (GtkTreeViewColumn *column,
                       GtkCellRenderer   *cell,
                       GtkTreeModel      *model,
                       GtkTreeIter       *iter)
{
  gboolean active;
  GdkPixbuf *icon_pixbuf;
  gchar *icon_name;

  gtk_tree_model_get (model, iter,
    PEAS_GTK_PLUGIN_STORE_MODEL_ACTIVE_COLUMN,      &active,
    PEAS_GTK_PLUGIN_STORE_MODEL_ICON_PIXBUF_COLUMN, &icon_pixbuf,
    PEAS_GTK_PLUGIN_STORE_MODEL_ICON_NAME_COLUMN,   &icon_name,
    -1);

  g_object_set (cell, "visible", !active, NULL);

  if (!active)
    {
      if (icon_pixbuf == NULL)
        g_object_set (cell, "icon-name", icon_name, NULL);
      else
        g_object_set (cell, "pixbuf", icon_pixbuf, NULL);
    }

  g_free (icon_name);

  if (icon_pixbuf != NULL)
    g_object_unref (icon_pixbuf);
}

static void
peas_gtk_plugin_store_init (PeasGtkPluginStore *store)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;

  store->priv = G_TYPE_INSTANCE_GET_PRIVATE (store,
                                             PEAS_GTK_TYPE_PLUGIN_STORE,
                                             PeasGtkPluginStorePrivate);

  /*gtk_widget_set_has_tooltip (GTK_WIDGET (store), TRUE);*/

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (store), TRUE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (store), FALSE);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("Plugin"));
  gtk_tree_view_column_set_resizable (column, FALSE);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  g_object_set (cell, "stock-size", GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
  gtk_tree_view_column_set_cell_data_func (column, cell,
                                           (GtkTreeCellDataFunc) plugin_icon_data_func,
                                           NULL, NULL);

  cell = gtk_cell_renderer_progress_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "visible", PEAS_GTK_PLUGIN_STORE_MODEL_ACTIVE_COLUMN,
                                       "pulse", PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN,
                                       "value", PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_COLUMN,
                                       NULL);

  gtk_tree_view_column_set_spacing (column, 6);
  gtk_tree_view_append_column (GTK_TREE_VIEW (store), column);

  column = gtk_tree_view_column_new ();

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, TRUE);
  g_object_set (cell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "markup", PEAS_GTK_PLUGIN_STORE_MODEL_INFO_COLUMN,
                                       NULL);

  gtk_tree_view_column_set_spacing (column, 6);
  gtk_tree_view_append_column (GTK_TREE_VIEW (store), column);

  /* Enable search for our non-string column */
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (store),
                                   PEAS_GTK_PLUGIN_STORE_MODEL_PLUGIN_COLUMN);
  gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (store),
                                       (GtkTreeViewSearchEqualFunc) name_search_cb,
                                       store,
                                       NULL);

  gtk_widget_show (GTK_WIDGET (store));
}

static gboolean
peas_gtk_plugin_store_button_press_event (GtkWidget      *view,
                                          GdkEventButton *event)
{
  PeasGtkPluginStore *store = PEAS_GTK_PLUGIN_STORE (view);
  GtkWidgetClass *widget_class;
  gboolean handled;

  widget_class = GTK_WIDGET_CLASS (peas_gtk_plugin_store_parent_class);

  /* The selection must by updated */
  handled = widget_class->button_press_event (view, event);

  if (event->type != GDK_BUTTON_PRESS || event->button != 3 || !handled)
    return handled;

  return show_popup_menu (GTK_TREE_VIEW (view), store, event);
}

static gboolean
peas_gtk_plugin_store_popup_menu (GtkWidget *view)
{
  PeasGtkPluginStore *store = PEAS_GTK_PLUGIN_STORE (view);

  return show_popup_menu (GTK_TREE_VIEW (view), store, NULL);
}

#if 0
static gboolean
peas_gtk_plugin_store_query_tooltip (GtkWidget  *widget,
                                    gint        x,
                                    gint        y,
                                    gboolean    keyboard_mode,
                                    GtkTooltip *tooltip)
{
  PeasGtkPluginStore *store = PEAS_GTK_PLUGIN_STORE (widget);
  gboolean is_row;
  GtkTreeIter iter;
  PeasGtkInstallablePluginInfo *info;
  GError *error = NULL;

  is_row = gtk_tree_view_get_tooltip_context (GTK_TREE_VIEW (widget),
                                              &x, &y, keyboard_mode,
                                              NULL, NULL, &iter);

  if (!is_row)
    return FALSE;

  info = peas_gtk_plugin_store_model_get_plugin (store->priv->model, &iter);

  if (!peas_gtk_installable_plugin_info_is_available (info, &error))
    {
      gchar *message;

      message = g_markup_printf_escaped (_("<b>The plugin '%s' could not be "
                                           "loaded</b>\nAn error occurred: %s"),
                                         peas_gtk_installable_plugin_info_get_name (info),
                                         error->message);

      gtk_tooltip_set_markup (tooltip, message);

      g_free (message);
      g_error_free (error);

      return TRUE;
    }

  return FALSE;
}
#endif

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

  store->priv->backend = peas_gtk_plugin_store_backend_xml_new (store->priv->engine);

  store->priv->model = peas_gtk_plugin_store_model_new (store->priv->engine);
  gtk_tree_view_set_model (GTK_TREE_VIEW (store),
                           GTK_TREE_MODEL (store->priv->model));
  g_object_unref (store->priv->model);

  store->priv->get_plugins_cancellable = g_cancellable_new ();
  peas_gtk_plugin_store_backend_get_plugins (store->priv->backend,
                                             store->priv->get_plugins_cancellable,
                                             (GAsyncReadyCallback) get_plugins_cb,
                                             g_object_ref (store));
  g_object_unref (store->priv->get_plugins_cancellable);

  if (G_OBJECT_CLASS (peas_gtk_plugin_store_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (peas_gtk_plugin_store_parent_class)->constructed (object);
}

static void
peas_gtk_plugin_store_dispose (GObject *object)
{
  PeasGtkPluginStore *store = PEAS_GTK_PLUGIN_STORE (object);

  if (store->priv->engine != NULL)
    {
      g_object_unref (store->priv->engine);
      store->priv->engine = NULL;
    }

  if (store->priv->backend != NULL)
    {
      g_object_unref (store->priv->backend);
      store->priv->backend = NULL;
    }

  if (store->priv->get_plugins_cancellable != NULL)
    {
      g_cancellable_cancel (store->priv->get_plugins_cancellable);
      store->priv->get_plugins_cancellable = NULL;
    }

  G_OBJECT_CLASS (peas_gtk_plugin_store_parent_class)->dispose (object);
}

static void
peas_gtk_plugin_store_class_init (PeasGtkPluginStoreClass *klass)
{
  GType the_type = G_TYPE_FROM_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = peas_gtk_plugin_store_set_property;
  object_class->get_property = peas_gtk_plugin_store_get_property;
  object_class->constructed = peas_gtk_plugin_store_constructed;
  object_class->dispose = peas_gtk_plugin_store_dispose;

  widget_class->button_press_event = peas_gtk_plugin_store_button_press_event;
  widget_class->popup_menu = peas_gtk_plugin_store_popup_menu;
#if 0
  widget_class->query_tooltip = peas_gtk_plugin_store_query_tooltip;
#endif

  /**
   * PeasGtkPluginStore:engine:
   *
   * The #PeasEngine this store is attached to.
   */
  g_object_class_install_property (object_class,
                                   PROP_ENGINE,
                                   g_param_spec_object ("engine",
                                                        "engine",
                                                        "The PeasEngine this store is attached to",
                                                        PEAS_TYPE_ENGINE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  /**
   * PeasGtkPluginStore::populate-popup:
   * @store: A #PeasGtkPluginStore.
   * @menu: A #GtkMenu.
   *
   * The populate-popup signal is emitted before showing the context
   * menu of the store. If you need to add items to the context menu,
   * connect to this signal and add your menuitems to the @menu.
   */
  signals[POPULATE_POPUP] =
    g_signal_new ("populate-popup",
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasGtkPluginStoreClass, populate_popup),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GTK_TYPE_MENU);

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

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (store));
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

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (store));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    info = peas_gtk_plugin_store_model_get_plugin (store->priv->model, &iter);

  return info;
}

/**
 * peas_gtk_plugin_store_install_plugin:
 * @store: A #PeasGtkPluginStore.
 *
 * Installs the currently selected plugin.
 */
void
peas_gtk_plugin_store_install_plugin (PeasGtkPluginStore *store)
{
  PeasGtkInstallablePluginInfo *info;
  ProgressData *progress;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE (store));

  info = peas_gtk_plugin_store_get_selected_plugin (store);
  g_return_if_fail (peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_installed (info));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_in_use (info));

  /* For now just leak the damn progress */
  progress = start_progress (store, info);

  peas_gtk_plugin_store_backend_install_plugin (store->priv->backend,
                                                info,
                                                NULL,
                                                (PeasGtkPluginStoreProgressCallback) plugin_progress_cb,
                                                progress,
                                                (GAsyncReadyCallback) install_plugin_cb,
                                                g_object_ref (store));
}

/**
 * peas_gtk_plugin_store_uninstall_plugin:
 * @store: A #PeasGtkPluginStore.
 *
 * Uninstalls the currently selected plugin.
 */
void
peas_gtk_plugin_store_uninstall_plugin (PeasGtkPluginStore *store)
{
  PeasGtkInstallablePluginInfo *info;
  ProgressData *progress;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE (store));

  info = peas_gtk_plugin_store_get_selected_plugin (store);
  g_return_if_fail (peas_gtk_installable_plugin_info_is_available (info, NULL));
  g_return_if_fail (peas_gtk_installable_plugin_info_is_installed (info));
  g_return_if_fail (!peas_gtk_installable_plugin_info_is_in_use (info));

  /* For now just leak the damn progress */
  progress = start_progress (store, info);

  peas_gtk_plugin_store_backend_uninstall_plugin (store->priv->backend,
                                                  info,
                                                  NULL,
                                                  (PeasGtkPluginStoreProgressCallback) plugin_progress_cb,
                                                  progress,
                                                  (GAsyncReadyCallback) uninstall_plugin_cb,
                                                  g_object_ref (store));
}
