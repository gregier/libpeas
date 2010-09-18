/*
 * peas-plugin-manager-view.c
 * This file is part of libpeas
 *
 * Copyright (C) 2002 Paolo Maggi and James Willcox
 * Copyright (C) 2003-2006 Paolo Maggi, Paolo Borelli
 * Copyright (C) 2007-2009 Paolo Maggi, Paolo Borelli, Steve Frécinaux
 * Copyright (C) 2010 Garrett Regier
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

#include <libpeas/peas-i18n.h>

#include "peas-gtk-plugin-manager-view.h"
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
 * to use the view to configure your plugins is to instantiate it using
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
  PROP_SHOW_BUILTIN
};

/* Signals */
enum {
  POPULATE_POPUP,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

G_DEFINE_TYPE (PeasGtkPluginManagerView, peas_gtk_plugin_manager_view, GTK_TYPE_TREE_VIEW);

static void
convert_iter_to_child_iter (PeasGtkPluginManagerView *view,
                            GtkTreeIter              *iter)
{
  if (!view->priv->show_builtin)
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
  gboolean success = TRUE;

  if (!view->priv->show_builtin)
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

static void
plugin_list_changed_cb (PeasEngine               *engine,
                        GParamSpec               *pspec,
                        PeasGtkPluginManagerView *view)
{
  PeasPluginInfo *info;

  info = peas_gtk_plugin_manager_view_get_selected_plugin (view);

  peas_gtk_plugin_manager_store_reload (view->priv->store);

  if (info == NULL)
    {
      GtkTreeModel *model;
      GtkTreeIter iter;

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

      if (gtk_tree_model_get_iter_first (model, &iter))
        {
          PeasGtkPluginManagerStore *store;

          store = PEAS_GTK_PLUGIN_MANAGER_STORE (view->priv->store);

          convert_iter_to_child_iter (view, &iter);
          info = peas_gtk_plugin_manager_store_get_plugin (store, &iter);
        }
    }

  if (info != NULL)
    peas_gtk_plugin_manager_view_set_selected_plugin (view, info);
}

static gboolean
filter_builtins_visible (PeasGtkPluginManagerStore *store,
                         GtkTreeIter               *iter,
                         PeasGtkPluginManagerView  *view)
{
  PeasPluginInfo *info;

  /* We never filter showing builtins */
  g_assert (view->priv->show_builtin == FALSE);

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
      peas_gtk_plugin_manager_store_toggle_enabled (view->priv->store, &iter);
    }

  gtk_tree_path_free (path);
}

static void
row_activated_cb (GtkTreeView              *tree_view,
                  GtkTreePath              *path,
                  GtkTreeViewColumn        *column,
                  PeasGtkPluginManagerView *view)
{
  GtkTreeIter iter;

  g_return_if_fail (gtk_tree_model_get_iter (gtk_tree_view_get_model (tree_view),
                                             &iter, path));

  convert_iter_to_child_iter (view, &iter);

  if (peas_gtk_plugin_manager_store_can_enable (view->priv->store, &iter))
    peas_gtk_plugin_manager_store_toggle_enabled (view->priv->store, &iter);
}

/* Callback used as the interactive search comparison function */
static gboolean
name_search_cb (GtkTreeModel             *model,
                gint                      column,
                const gchar              *key,
                GtkTreeIter              *iter,
                PeasGtkPluginManagerView *view)
{
  PeasPluginInfo *info;
  gchar *normalized_string;
  gchar *normalized_key;
  gchar *case_normalized_string;
  gchar *case_normalized_key;
  gint key_len;
  gboolean retval;

  info = peas_gtk_plugin_manager_store_get_plugin (view->priv->store, iter);

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

  peas_gtk_plugin_manager_store_toggle_enabled (view->priv->store, &iter);
}

static void
enable_all_menu_cb (GtkMenu                  *menu,
                    PeasGtkPluginManagerView *view)
{
  peas_gtk_plugin_manager_store_set_all_enabled (view->priv->store, TRUE);
}

static void
disable_all_menu_cb (GtkMenu                  *menu,
                     PeasGtkPluginManagerView *view)
{
  peas_gtk_plugin_manager_store_set_all_enabled (view->priv->store, FALSE);
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
  gtk_widget_set_sensitive (item, peas_plugin_info_is_available (info) &&
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
  view->priv->popup_menu = NULL;
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
#if !GTK_CHECK_VERSION(2,90,7)
          gtk_widget_size_request (GTK_WIDGET (menu), &requisition);
#else
          gtk_size_request_get_size (GTK_SIZE_REQUEST (menu), &requisition,
                                     NULL);
#endif
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
#if !GTK_CHECK_VERSION(2,90,7)
          gtk_widget_size_request (GTK_WIDGET (menu), &requisition);
#else
          gtk_size_request_get_size (GTK_SIZE_REQUEST (menu), &requisition,
                                     NULL);
#endif
          *x += allocation.width - requisition.width;
        }
    }

  *push_in = TRUE;
}

static void
show_popup_menu (GtkTreeView              *tree_view,
                 PeasGtkPluginManagerView *view,
                 GdkEventButton           *event)
{
  if (view->priv->popup_menu)
    gtk_widget_destroy (view->priv->popup_menu);

  view->priv->popup_menu = create_popup_menu (view);

  if (view->priv->popup_menu == NULL)
    return;

  gtk_menu_attach_to_widget (GTK_MENU (view->priv->popup_menu),
                             GTK_WIDGET (view),
                             (GtkMenuDetachFunc) popup_menu_detach);

  if (event != NULL)
    {
      gtk_menu_popup (GTK_MENU (view->priv->popup_menu), NULL, NULL,
                      NULL, NULL, event->button, event->time);
    }
  else
    {
      gtk_menu_popup (GTK_MENU (view->priv->popup_menu), NULL, NULL,
                      (GtkMenuPositionFunc) menu_position_under_tree_view,
                      view, 0, gtk_get_current_event_time ());

      gtk_menu_shell_select_first (GTK_MENU_SHELL (view->priv->popup_menu),
                                   FALSE);
    }
}

static gboolean
button_press_event_cb (GtkWidget                *tree_view,
                       GdkEventButton           *event,
                       PeasGtkPluginManagerView *view)
{
  GtkWidgetClass *widget_class;
  gboolean handled;

  if (event->type != GDK_BUTTON_PRESS || event->button != 3)
    return FALSE;

  widget_class = GTK_WIDGET_CLASS (peas_gtk_plugin_manager_view_parent_class);

  /* The selection must by updated */
  handled = widget_class->button_press_event (tree_view, event);

  if (!handled)
    return FALSE;

  show_popup_menu (GTK_TREE_VIEW (tree_view), view, event);

  return TRUE;
}

static gboolean
popup_menu_cb (GtkTreeView              *tree_view,
               PeasGtkPluginManagerView *view)
{
  show_popup_menu (tree_view, view, NULL);

  return TRUE;
}

static void
peas_gtk_plugin_manager_view_init (PeasGtkPluginManagerView *view)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;

  view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view,
                                            PEAS_GTK_TYPE_PLUGIN_MANAGER_VIEW,
                                            PeasGtkPluginManagerViewPrivate);

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
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
  gtk_tree_view_column_set_attributes (column, cell,
                                       //"sensitive", PEAS_GTK_PLUGIN_MANAGER_STORE_CAN_ENABLE_COLUMN,
                                       "icon-name", PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_COLUMN,
                                       NULL);

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

  gtk_widget_show (GTK_WIDGET (view));

  g_signal_connect (view,
                    "row-activated",
                    G_CALLBACK (row_activated_cb),
                    view);
  g_signal_connect (view,
                    "button-press-event",
                    G_CALLBACK (button_press_event_cb),
                    view);
  g_signal_connect (view,
                    "popup-menu",
                    G_CALLBACK (popup_menu_cb),
                    view);
}

static void
peas_gtk_plugin_manager_view_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      view->priv->engine = g_value_get_object (value);
      g_object_ref (view->priv->engine);
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

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, view->priv->engine);
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

  view->priv->store = peas_gtk_plugin_manager_store_new (view->priv->engine);

  /* Properly set the model */
  view->priv->show_builtin = TRUE;
  peas_gtk_plugin_manager_view_set_show_builtin (view, FALSE);

  g_signal_connect (view->priv->engine,
                    "notify::plugin-list",
                    G_CALLBACK (plugin_list_changed_cb),
                    view);

  if (G_OBJECT_CLASS (peas_gtk_plugin_manager_view_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (peas_gtk_plugin_manager_view_parent_class)->constructed (object);
}

static void
peas_gtk_plugin_manager_view_dispose (GObject *object)
{
  PeasGtkPluginManagerView *view = PEAS_GTK_PLUGIN_MANAGER_VIEW (object);

  if (view->priv->popup_menu != NULL)
    {
      gtk_widget_destroy (view->priv->popup_menu);
      view->priv->popup_menu = NULL;
    }

  if (view->priv->store != NULL)
    {
      g_object_unref (view->priv->store);
      view->priv->store = NULL;
    }

  if (view->priv->engine != NULL)
    {
      g_signal_handlers_disconnect_by_func (view->priv->engine,
                                            plugin_list_changed_cb,
                                            view);

      g_object_unref (view->priv->engine);
      view->priv->engine = NULL;
    }

  G_OBJECT_CLASS (peas_gtk_plugin_manager_view_parent_class)->dispose (object);
}

static void
peas_gtk_plugin_manager_view_class_init (PeasGtkPluginManagerViewClass *klass)
{
  GType the_type = G_TYPE_FROM_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_gtk_plugin_manager_view_set_property;
  object_class->get_property = peas_gtk_plugin_manager_view_get_property;
  object_class->constructed = peas_gtk_plugin_manager_view_constructed;
  object_class->dispose = peas_gtk_plugin_manager_view_dispose;

  /**
   * PeasGtkPLuginManagerView:engine:
   *
   * The #PeasEngine this view is attached to.
   */
  g_object_class_install_property (object_class,
                                   PROP_ENGINE,
                                   g_param_spec_object ("engine",
                                                        "engine",
                                                        "The PeasEngine this view is attached to",
                                                        PEAS_TYPE_ENGINE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  /**
   * PeasGtkPLuginManagerView:show-builtin:
   *
   * If builtin plugins should be shown.
   */
  g_object_class_install_property (object_class,
                                   PROP_ENGINE,
                                   g_param_spec_boolean ("show-builtin",
                                                         "show-builtin",
                                                         "If builtin plugins should be shown",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

  /**
   * PeasGtkPluginManagerView::populate-popup:
   * @view: A #PeasGtkPluginManagerView.
   * @menu: A #GtkMenu.
   *
   * The populate-popup signal is emitted before showing the context
   * menu of the view. If you need to add items to the context menu,
   * connect to this signal and add your menuitems to the @menu.
   */
  signals[POPULATE_POPUP] =
    g_signal_new ("populate-popup",
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasGtkPluginManagerViewClass, populate_popup),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GTK_TYPE_MENU);

  g_type_class_add_private (object_class, sizeof (PeasGtkPluginManagerViewPrivate));
}

/**
 * peas_gtk_plugin_manager_view_new:
 * @engine: A #PeasEngine.
 *
 * Creates a new plugin manager view for the given #PeasEngine.
 *
 * Returns: the new #PeasGtkPluginManagerView.
 */
GtkWidget *
peas_gtk_plugin_manager_view_new (PeasEngine *engine)
{
  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);

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
 */
void
peas_gtk_plugin_manager_view_set_show_builtin (PeasGtkPluginManagerView *view,
                                               gboolean                 show_builtin)
{
  GtkTreeIter iter;
  gboolean iter_set;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view));

  show_builtin = (show_builtin != FALSE);

  if (view->priv->show_builtin == show_builtin)
    return;

  /* We must get the selected iter before setting if builtin
     plugins should be shown so the proper model is set */
  iter_set = peas_gtk_plugin_manager_view_get_selected_iter (view, &iter);

  view->priv->show_builtin = show_builtin;

  if (show_builtin == TRUE)
    {
      gtk_tree_view_set_model (GTK_TREE_VIEW (view),
                               GTK_TREE_MODEL (view->priv->store));
    }
  else
    {
      GtkTreeModel *model;

      model = gtk_tree_model_filter_new (GTK_TREE_MODEL (view->priv->store), NULL);
      gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (model),
                                              (GtkTreeModelFilterVisibleFunc) filter_builtins_visible,
                                              view,
                                              NULL);

      gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

      g_object_unref (model);
    }

  if (iter_set)
    peas_gtk_plugin_manager_view_set_selected_iter (view, &iter);
}

/**
 * peas_gtk_plugin_manager_view_get_show_builtin:
 * @view: A #PeasGtkPluginManagerView.
 *
 * Returns if builtin plugins should be shown.
 *
 * Returns: if builtin plugins should be shown.
 */
gboolean
peas_gtk_plugin_manager_view_get_show_builtin (PeasGtkPluginManagerView *view)
{
  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view), FALSE);

  return view->priv->show_builtin;
}

/**
 * peas_gtk_plugin_manager_view_set_selected_iter:
 * @view: A #PeasGtkPluginManagerView.
 * @iter: A #GtkTreeIter.
 *
 * Selects @iter.
 */
void
peas_gtk_plugin_manager_view_set_selected_iter (PeasGtkPluginManagerView *view,
                                                GtkTreeIter             *iter)
{
  GtkTreeSelection *selection;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view));
  g_return_if_fail (iter != NULL);

  if (!convert_child_iter_to_iter (view, iter))
    return;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  gtk_tree_selection_select_iter (selection, iter);
}

/**
 * peas_gtk_plugin_manager_view_get_selected_iter:
 * @view: A #PeasGtkPluginManagerView.
 * @iter: A #GtkTreeIter.
 *
 * Returns if @iter was set to the selected plugin.
 *
 * Returns: if @iter was set.
 */
gboolean
peas_gtk_plugin_manager_view_get_selected_iter (PeasGtkPluginManagerView *view,
                                                GtkTreeIter             *iter)
{
  GtkTreeSelection *selection;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

  if (!gtk_tree_selection_get_selected (selection, NULL, iter))
    return FALSE;

  convert_iter_to_child_iter (view, iter);

  return TRUE;
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
                                                  PeasPluginInfo          *info)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view));
  g_return_if_fail (info != NULL);

  g_return_if_fail (peas_gtk_plugin_manager_store_get_iter_from_plugin (view->priv->store,
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
 * Returns: the selected plugin.
 */
PeasPluginInfo *
peas_gtk_plugin_manager_view_get_selected_plugin (PeasGtkPluginManagerView *view)
{
  GtkTreeIter iter;
  PeasPluginInfo *info = NULL;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER_VIEW (view), NULL);

  if (peas_gtk_plugin_manager_view_get_selected_iter (view, &iter))
    info = peas_gtk_plugin_manager_store_get_plugin (view->priv->store, &iter);

  return info;
}
