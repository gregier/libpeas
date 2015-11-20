/*
 * peas-plugin-manager-view.c
 * This file is part of libpeas
 *
 * Copyright (C) 2002 Paolo Maggi and James Willcox
 * Copyright (C) 2003-2006 Paolo Maggi, Paolo Borelli
 * Copyright (C) 2007-2009 Paolo Maggi, Paolo Borelli, Steve Frécinaux
 * Copyright (C) 2010 Garrett Regier
 *
 * libpeas is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libpeas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <libpeas/peas-engine.h>
#include <libpeas/peas-i18n.h>

#include "peas-gtk-plugin-manager-view.h"
#include "peas-gtk-disable-plugins-dialog.h"
#include "peas-gtk-plugin-manager-store.h"
#include "peas-gtk-configurable.h"

/**
 * SECTION:peas-gtk-plugin-manager-view
 * @short_description: Management tree view for plugins.
 *
 * The #PeasGtkPluginManagerView is a tree view that can be used to manage
 * plugins, i.e. load or unload them, and see some pieces of information.
 *
 * The only thing you need to do as an application writer if you wish
 * to use the view to display your plugins is to instantiate it using
 * peas_gtk_plugin_manager_view_new() and pack it into another
 * widget or a window.
 *
 * Note: Changing the model of the view is not supported.
 *
 **/

struct _PeasGtkPluginManagerViewPrivate {
  PeasEngine *engine;

  PeasGtkPluginManagerStore *store;

  GtkWidget *popup_menu;

  guint show_builtin : 1;
};

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE,
  PROP_SHOW_BUILTIN,
  N_PROPERTIES
};

/* Signals */
enum {
  POPULATE_POPUP,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];
static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE (PeasGtkPluginManagerView,
                            peas_gtk_plugin_manager_view,
                            GTK_TYPE_TREE_VIEW)

#define GET_PRIV(o) \
  (peas_gtk_plugin_manager_view_get_instance_private (o))

static void
convert_iter_to_child_iter (PeasGtkPluginManagerView *view,
                            GtkTreeIter              *iter)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  if (!priv->show_builtin)
    {
      GtkTreeModel *model;
      GtkTreeIter child_iter;

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

      gtk_tree_model_filter_convert_iter_to_child_iter (GTK_TREE_MODEL_FILTER (model),
                                                        &child_iter, iter);

      *iter = child_iter;
    }
}

static gboolean
convert_child_iter_to_iter (PeasGtkPluginManagerView *view,
                            GtkTreeIter              *child_iter)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  gboolean success = TRUE;

  if (!priv->show_builtin)
    {
      GtkTreeModel *model;
      GtkTreeIter iter;

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

      success = gtk_tree_model_filter_convert_child_iter_to_iter (GTK_TREE_MODEL_FILTER (model),
                                                                  &iter, child_iter);

      if (success)
        *child_iter = iter;
    }

  return success;
}

static GList *
get_dependant_plugins (PeasGtkPluginManagerView *view,
                       PeasPluginInfo           *info)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  const gchar *module_name;
  const GList *plugins;
  GList *dep_plugins = NULL;

  module_name = peas_plugin_info_get_module_name (info);
  plugins = peas_engine_get_plugin_list (priv->engine);

  for (; plugins != NULL; plugins = plugins->next)
    {
      PeasPluginInfo *plugin = (PeasPluginInfo *) plugins->data;

      if (peas_plugin_info_is_hidden (plugin) ||
          !peas_plugin_info_is_loaded (plugin))
        continue;

      /* Don't add builtin plugins if they are not shown */
      if (!priv->show_builtin && peas_plugin_info_is_builtin (plugin))
        continue;

      if (peas_plugin_info_has_dependency (plugin, module_name))
        dep_plugins = g_list_prepend (dep_plugins, plugin);
    }

  return dep_plugins;
}

static void
toggle_enabled (PeasGtkPluginManagerView *view,
                GtkTreeIter              *iter)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  PeasPluginInfo *info;

  info = peas_gtk_plugin_manager_store_get_plugin (priv->store, iter);

  if (peas_plugin_info_is_loaded (info))
    {
      GList *dep_plugins;

      dep_plugins = get_dependant_plugins (view, info);

      if (dep_plugins != NULL)
        {
          GtkWindow *parent;
          GtkWidget *dialog;
          gint response;

          parent = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view)));

          /* The dialog takes the list so don't free it */
          dialog = peas_gtk_disable_plugins_dialog_new (parent, info,
                                                        dep_plugins);

          response = gtk_dialog_run (GTK_DIALOG (dialog));

          gtk_widget_destroy (dialog);

          if (response != GTK_RESPONSE_OK)
            return;
        }
    }

  peas_gtk_plugin_manager_store_toggle_enabled (priv->store, iter);
}

static void
plugin_list_changed_cb (PeasEngine               *engine,
                        GParamSpec               *pspec,
                        PeasGtkPluginManagerView *view)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  PeasPluginInfo *info;

  info = peas_gtk_plugin_manager_view_get_selected_plugin (view);

  peas_gtk_plugin_manager_store_reload (priv->store);

  if (info != NULL)
    peas_gtk_plugin_manager_view_set_selected_plugin (view, info);
}

static gboolean
filter_builtins_visible (PeasGtkPluginManagerStore *store,
                         GtkTreeIter               *iter,
                         PeasGtkPluginManagerView  *view)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  PeasPluginInfo *info;

  /* We never filter showing builtins */
  g_assert (priv->show_builtin == FALSE);

  info = peas_gtk_plugin_manager_store_get_plugin (store, iter);

  if (info == NULL)
    return FALSE;

  return !peas_plugin_info_is_builtin (info);
}

static void
enabled_toggled_cb (GtkCellRendererToggle    *cell,
                    gchar                    *path_str,
                    PeasGtkPluginManagerView *view)
{
  GtkTreeModel *model;
  GtkTreePath *path;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));
  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (model, &iter, path))
    {
      convert_iter_to_child_iter (view, &iter);
      toggle_enabled (view, &iter);
    }

  gtk_tree_path_free (path);
}

/* Callback used as the interactive search comparison function */
static gboolean
name_search_cb (GtkTreeModel             *model,
                gint                      column,
                const gchar              *key,
                GtkTreeIter              *iter,
                PeasGtkPluginManagerView *view)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  GtkTreeIter child_iter = *iter;
  PeasPluginInfo *info;
  gchar *normalized_string;
  gchar *normalized_key;
  gchar *case_normalized_string;
  gchar *case_normalized_key;
  gint key_len;
  gboolean retval;

  convert_iter_to_child_iter (view, &child_iter);
  info = peas_gtk_plugin_manager_store_get_plugin (priv->store, &child_iter);

  if (info == NULL)
    return FALSE;

  normalized_string = g_utf8_normalize (peas_plugin_info_get_name (info), -1, G_NORMALIZE_ALL);
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
enabled_menu_cb (GtkMenu                  *menu,
                 PeasGtkPluginManagerView *view)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

  g_return_if_fail (gtk_tree_selection_get_selected (selection, NULL, &iter));

  convert_iter_to_child_iter (view, &iter);

  toggle_enabled (view, &iter);
}

static void
enable_all_menu_cb (GtkMenu                  *menu,
                    PeasGtkPluginManagerView *view)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  peas_gtk_plugin_manager_store_set_all_enabled (priv->store, TRUE);
}

static void
disable_all_menu_cb (GtkMenu                  *menu,
                     PeasGtkPluginManagerView *view)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  peas_gtk_plugin_manager_store_set_all_enabled (priv->store, FALSE);
}

static GtkWidget *
create_popup_menu (PeasGtkPluginManagerView *view)
{
  PeasPluginInfo *info;
  GtkWidget *menu;
  GtkWidget *item;

  info = peas_gtk_plugin_manager_view_get_selected_plugin (view);

  if (info == NULL)
    return NULL;

  menu = gtk_menu_new ();

  item = gtk_check_menu_item_new_with_mnemonic (_("_Enabled"));
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                  peas_plugin_info_is_loaded (info));
  g_signal_connect (item, "toggled", G_CALLBACK (enabled_menu_cb), view);
  gtk_widget_set_sensitive (item, peas_plugin_info_is_available (info, NULL) &&
                                  !peas_plugin_info_is_builtin (info));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_menu_item_new_with_mnemonic (_("E_nable All"));
  g_signal_connect (item, "activate", G_CALLBACK (enable_all_menu_cb), view);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_menu_item_new_with_mnemonic (_("_Disable All"));
  g_signal_connect (item, "activate", G_CALLBACK (disable_all_menu_cb), view);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  g_signal_emit (view, signals[POPULATE_POPUP], 0, menu);

  gtk_widget_show_all (menu);

  return menu;
}

static void
popup_menu_detach (PeasGtkPluginManagerView *view,
                   GtkMenu                  *menu)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  priv->popup_menu = NULL;
}

static void
menu_position_under_tree_view (GtkMenu     *menu,
                               gint        *x,
                               gint        *y,
                               gboolean    *push_in,
                               GtkTreeView *tree_view)
{
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GdkWindow *window;

  selection = gtk_tree_view_get_selection (tree_view);

  window = gtk_widget_get_window (GTK_WIDGET (tree_view));
  gdk_window_get_origin (window, x, y);

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      GtkTreeModel *model;
      GtkTreePath *path;
      GdkRectangle rect;

      model = gtk_tree_view_get_model (tree_view);
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_view_get_cell_area (tree_view,
                                   path,
                                   gtk_tree_view_get_column (tree_view, 0), /* FIXME 0 for RTL ? */
                                   &rect);
      gtk_tree_path_free (path);

      *x += rect.x;
      *y += rect.y + rect.height;

      if (gtk_widget_get_direction (GTK_WIDGET (tree_view)) == GTK_TEXT_DIR_RTL)
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

      gtk_widget_get_allocation (GTK_WIDGET (tree_view), &allocation);

      *x += allocation.x;
      *y += allocation.y;

      if (gtk_widget_get_direction (GTK_WIDGET (tree_view)) == GTK_TEXT_DIR_RTL)
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
show_popup_menu (GtkTreeView              *tree_view,
                 PeasGtkPluginManagerView *view,
                 GdkEventButton           *event)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  if (priv->popup_menu)
    gtk_widget_destroy (priv->popup_menu);

  priv->popup_menu = create_popup_menu (view);

  if (priv->popup_menu == NULL)
    return FALSE;

  gtk_menu_attach_to_widget (GTK_MENU (priv->popup_menu),
                             GTK_WIDGET (view),
                             (GtkMenuDetachFunc) popup_menu_detach);

  if (event != NULL)
    {
      gtk_menu_popup (GTK_MENU (priv->popup_menu), NULL, NULL,
                      NULL, NULL, event->button, event->time);
    }
  else
    {
      gtk_menu_popup (GTK_MENU (priv->popup_menu), NULL, NULL,
                      (GtkMenuPositionFunc) menu_position_under_tree_view,
                      view, 0, gtk_get_current_event_time ());

      gtk_menu_shell_select_first (GTK_MENU_SHELL (priv->popup_menu),
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
  GIcon *icon_gicon;
  gchar *icon_stock_id;

  gtk_tree_model_get (model, iter,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_GICON_COLUMN, &icon_gicon,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_STOCK_ID_COLUMN, &icon_stock_id,
    -1);

  if (icon_gicon == NULL)
    {
      g_object_set (cell, "stock-id", icon_stock_id, NULL);
    }
  else
    {
      g_object_set (cell, "gicon", icon_gicon, NULL);
      g_object_unref (icon_gicon);
    }

  g_free (icon_stock_id);
}

static void
peas_gtk_plugin_manager_view_init (PeasGtkPluginManagerView *view)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;

  gtk_widget_set_has_tooltip (GTK_WIDGET (view), TRUE);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);

  /* first column */
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("Enabled"));
  gtk_tree_view_column_set_resizable (column, FALSE);

  cell = gtk_cell_renderer_toggle_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  g_object_set (cell, "xpad", 6, NULL);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "active", PEAS_GTK_PLUGIN_MANAGER_STORE_ENABLED_COLUMN,
                                       "activatable", PEAS_GTK_PLUGIN_MANAGER_STORE_CAN_ENABLE_COLUMN,
                                       "sensitive", PEAS_GTK_PLUGIN_MANAGER_STORE_CAN_ENABLE_COLUMN,
                                       "visible", PEAS_GTK_PLUGIN_MANAGER_STORE_CAN_ENABLE_COLUMN,
                                       NULL);
  g_signal_connect (cell,
                    "toggled",
                    G_CALLBACK (enabled_toggled_cb),
                    view);

  gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

  /* second column */
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("Plugin"));
  gtk_tree_view_column_set_resizable (column, FALSE);

  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  g_object_set (cell, "stock-size", GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
  gtk_tree_view_column_set_cell_data_func (column, cell,
                                           (GtkTreeCellDataFunc) plugin_icon_data_func,
                                           NULL, NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, TRUE);
  g_object_set (cell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  gtk_tree_view_column_set_attributes (column, cell,
                                       "sensitive", PEAS_GTK_PLUGIN_MANAGER_STORE_INFO_SENSITIVE_COLUMN,
                                       "markup", PEAS_GTK_PLUGIN_MANAGER_STORE_INFO_COLUMN,
                                       NULL);

  gtk_tree_view_column_set_spacing (column, 6);
  gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

  /* Enable search for our non-string column */
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (view),
                                   PEAS_GTK_PLUGIN_MANAGER_STORE_PLUGIN_COLUMN);
  gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (view),
                                       (GtkTreeViewSearchEqualFunc) name_search_cb,
                                       view,
                                       NULL);
}

static gboolean
peas_gtk_plugin_manager_view_button_press_event (GtkWidget      *tree_view,
                                                 GdkEventButton *event)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (tree_view);
  GtkWidgetClass *widget_class;
  gboolean handled;

  widget_class = GTK_WIDGET_CLASS (peas_gtk_plugin_manager_view_parent_class);

  /* The selection must by updated */
  handled = widget_class->button_press_event (tree_view, event);

  if (event->type != GDK_BUTTON_PRESS || event->button != 3 || !handled)
    return handled;

  return show_popup_menu (GTK_TREE_VIEW (tree_view), view, event);
}

static gboolean
peas_gtk_plugin_manager_view_popup_menu (GtkWidget *tree_view)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (tree_view);

  return show_popup_menu (GTK_TREE_VIEW (tree_view), view, NULL);
}

static gboolean
peas_gtk_plugin_manager_view_query_tooltip (GtkWidget  *widget,
                                            gint        x,
                                            gint        y,
                                            gboolean    keyboard_mode,
                                            GtkTooltip *tooltip)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (widget);
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  gboolean is_row;
  GtkTreeIter iter;
  PeasPluginInfo *info;
  gchar *to_bold, *error_msg, *message;
  GError *error = NULL;

  is_row = gtk_tree_view_get_tooltip_context (GTK_TREE_VIEW (widget),
                                              &x, &y, keyboard_mode,
                                              NULL, NULL, &iter);

  if (!is_row)
    return FALSE;

  convert_iter_to_child_iter (view, &iter);

  info = peas_gtk_plugin_manager_store_get_plugin (priv->store, &iter);

  if (peas_plugin_info_is_available (info, &error))
    return FALSE;

  /* Avoid having markup in a translated string */
  to_bold = g_strdup_printf (_("The plugin '%s' could not be loaded"),
                             peas_plugin_info_get_name (info));

  /* Keep separate because some translations do special things
   * for the ':' and might accidentally not keep the space after it
   */
  error_msg = g_strdup_printf (_("An error occurred: %s"), error->message);

  message = g_strconcat ("<b>", to_bold, "</b>\n", error_msg, NULL);

  gtk_tooltip_set_markup (tooltip, message);

  g_free (message);
  g_free (error_msg);
  g_free (to_bold);
  g_error_free (error);

  return TRUE;
}

static void
peas_gtk_plugin_manager_view_row_activated (GtkTreeView       *tree_view,
                                            GtkTreePath       *path,
                                            GtkTreeViewColumn *column)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (tree_view);
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  GtkTreeIter iter;
  GtkTreeViewClass *tree_view_class;

  if (!gtk_tree_model_get_iter (gtk_tree_view_get_model (tree_view), &iter, path))
    return;

  convert_iter_to_child_iter (view, &iter);

  if (peas_gtk_plugin_manager_store_can_enable (priv->store, &iter))
    toggle_enabled (view, &iter);

  tree_view_class = GTK_TREE_VIEW_CLASS (peas_gtk_plugin_manager_view_parent_class);
  if (tree_view_class->row_activated != NULL)
    tree_view_class->row_activated (tree_view, path, column);
}

static void
peas_gtk_plugin_manager_view_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (object);
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  switch (prop_id)
    {
    case PROP_ENGINE:
      priv->engine = g_value_get_object (value);
      break;
    case PROP_SHOW_BUILTIN:
      peas_gtk_plugin_manager_view_set_show_builtin (view,
                                                     g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_plugin_manager_view_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (object);
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, priv->engine);
      break;
    case PROP_SHOW_BUILTIN:
      g_value_set_boolean (value,
                           peas_gtk_plugin_manager_view_get_show_builtin (view));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_plugin_manager_view_constructed (GObject *object)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (object);
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  if (priv->engine == NULL)
    priv->engine = peas_engine_get_default ();

  g_object_ref (priv->engine);

  priv->store = peas_gtk_plugin_manager_store_new (priv->engine);

  /* Properly set the model */
  priv->show_builtin = TRUE;
  peas_gtk_plugin_manager_view_set_show_builtin (view, FALSE);

  g_signal_connect_object (priv->engine,
                           "notify::plugin-list",
                           G_CALLBACK (plugin_list_changed_cb),
                           view,
                           0);

  G_OBJECT_CLASS (peas_gtk_plugin_manager_view_parent_class)->constructed (object);
}

static void
peas_gtk_plugin_manager_view_dispose (GObject *object)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (object);
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  g_clear_pointer (&priv->popup_menu,
                   (GDestroyNotify) gtk_widget_destroy);

  g_clear_object (&priv->engine);
  g_clear_object (&priv->store);

  G_OBJECT_CLASS (peas_gtk_plugin_manager_view_parent_class)->dispose (object);
}

static void
peas_gtk_plugin_manager_view_class_init (PeasGtkPluginManagerViewClass *klass)
{
  GType the_type = G_TYPE_FROM_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkTreeViewClass *tree_view_class = GTK_TREE_VIEW_CLASS (klass);

  object_class->set_property = peas_gtk_plugin_manager_view_set_property;
  object_class->get_property = peas_gtk_plugin_manager_view_get_property;
  object_class->constructed = peas_gtk_plugin_manager_view_constructed;
  object_class->dispose = peas_gtk_plugin_manager_view_dispose;

  widget_class->button_press_event = peas_gtk_plugin_manager_view_button_press_event;
  widget_class->popup_menu = peas_gtk_plugin_manager_view_popup_menu;
  widget_class->query_tooltip = peas_gtk_plugin_manager_view_query_tooltip;

  tree_view_class->row_activated = peas_gtk_plugin_manager_view_row_activated;

  /**
   * PeasGtkPLuginManagerView:engine:
   *
   * The #PeasEngine this view is attached to.
   */
  properties[PROP_ENGINE] =
    g_param_spec_object ("engine",
                         "engine",
                         "The PeasEngine this view is attached to",
                         PEAS_TYPE_ENGINE,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * PeasGtkPluginManagerView:show-builtin:
   *
   * If builtin plugins should be shown.
   *
   * Deprecated: 1.2: Use hidden plugins instead.
   */
  properties[PROP_SHOW_BUILTIN] =
    g_param_spec_boolean ("show-builtin",
                          "show-builtin",
                          "If builtin plugins should be shown",
                          FALSE,
                          G_PARAM_DEPRECATED |
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS);

  /**
   * PeasGtkPluginManagerView::populate-popup:
   * @view: A #PeasGtkPluginManagerView.
   * @menu: A #GtkMenu.
   *
   * The ::populate-popup signal is emitted before showing the context
   * menu of the view. If you need to add items to the context menu,
   * connect to this signal and add your menuitems to the @menu.
   */
  signals[POPULATE_POPUP] =
    g_signal_new (I_("populate-popup"),
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasGtkPluginManagerViewClass, populate_popup),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GTK_TYPE_MENU);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

/**
 * peas_gtk_plugin_manager_view_new:
 * @engine: (allow-none): A #PeasEngine, or %NULL.
 *
 * Creates a new plugin manager view for the given #PeasEngine.
 *
 * If @engine is %NULL, then the default engine will be used.
 *
 * Returns: the new #PeasGtkPluginManagerView.
 */
GtkWidget *
peas_gtk_plugin_manager_view_new (PeasEngine *engine)
{
  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);

  return GTK_WIDGET (g_object_new (PEAS_GTK_TYPE_PLUGIN_MANAGER_VIEW,
                                   "engine", engine,
                                   NULL));
}

/**
 * peas_gtk_plugin_manager_view_set_show_builtin:
 * @view: A #PeasGtkPluginManagerView.
 * @show_builtin: If builtin plugins should be shown.
 *
 * Sets if builtin plugins should be shown.
 *
 * Deprecated: 1.2: Use hidden plugins instead.
 */
void
peas_gtk_plugin_manager_view_set_show_builtin (PeasGtkPluginManagerView *view,
                                               gboolean                  show_builtin)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  gboolean iter_set;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view));

  show_builtin = (show_builtin != FALSE);

  if (priv->show_builtin == show_builtin)
    return;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

  /* We must get the selected iter before setting if builtin
     plugins should be shown so the proper model is set */
  iter_set = gtk_tree_selection_get_selected (selection, NULL, &iter);

  if (iter_set)
    convert_iter_to_child_iter (view, &iter);

  priv->show_builtin = show_builtin;

  if (show_builtin)
    {
      gtk_tree_view_set_model (GTK_TREE_VIEW (view),
                               GTK_TREE_MODEL (priv->store));
    }
  else
    {
      GtkTreeModel *model;

      model = gtk_tree_model_filter_new (GTK_TREE_MODEL (priv->store), NULL);
      gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (model),
                                              (GtkTreeModelFilterVisibleFunc) filter_builtins_visible,
                                              view,
                                              NULL);

      gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

      g_object_unref (model);
    }

  if (iter_set && convert_child_iter_to_iter (view, &iter))
    gtk_tree_selection_select_iter (selection, &iter);

  g_object_notify_by_pspec (G_OBJECT (view),
                            properties[PROP_SHOW_BUILTIN]);
}

/**
 * peas_gtk_plugin_manager_view_get_show_builtin:
 * @view: A #PeasGtkPluginManagerView.
 *
 * Returns if builtin plugins should be shown.
 *
 * Returns: if builtin plugins should be shown.
 *
 * Deprecated: 1.2: Use hidden plugins instead.
 */
gboolean
peas_gtk_plugin_manager_view_get_show_builtin (PeasGtkPluginManagerView *view)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view), FALSE);

  return priv->show_builtin;
}

/**
 * peas_gtk_plugin_manager_view_set_selected_plugin:
 * @view: A #PeasGtkPluginManagerView.
 * @info: A #PeasPluginInfo.
 *
 * Selects the given plugin.
 */
void
peas_gtk_plugin_manager_view_set_selected_plugin (PeasGtkPluginManagerView *view,
                                                  PeasPluginInfo           *info)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  GtkTreeIter iter;
  GtkTreeSelection *selection;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view));
  g_return_if_fail (info != NULL);

  g_return_if_fail (peas_gtk_plugin_manager_store_get_iter_from_plugin (priv->store,
                                                                        &iter, info));

  if (!convert_child_iter_to_iter (view, &iter))
    return;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  gtk_tree_selection_select_iter (selection, &iter);
}

/**
 * peas_gtk_plugin_manager_view_get_selected_plugin:
 * @view: A #PeasGtkPluginManagerView.
 *
 * Returns the currently selected plugin, or %NULL if a plugin is not selected.
 *
 * Returns: (transfer none): the selected plugin.
 */
PeasPluginInfo *
peas_gtk_plugin_manager_view_get_selected_plugin (PeasGtkPluginManagerView *view)
{
  PeasGtkPluginManagerViewPrivate *priv = GET_PRIV (view);
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  PeasPluginInfo *info = NULL;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

  /* Since gtk+ 3.4 gtk_tree_view_get_selection() can in practice return NULL
   * here because 'cursor-changed' is emitted during 'destroy' (it wasn't
   * the case previously and is not properly documented as of today).
   */
  if (selection != NULL && gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      convert_iter_to_child_iter (view, &iter);
      info = peas_gtk_plugin_manager_store_get_plugin (priv->store, &iter);
    }

  return info;
}
