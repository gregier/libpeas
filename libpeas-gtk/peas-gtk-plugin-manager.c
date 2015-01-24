/*
 * peas-plugin-manager.c
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
#include <girepository.h>

#ifdef OS_OSX
#import <Cocoa/Cocoa.h>
#endif

#include <libpeas/peas-engine.h>
#include <libpeas/peas-plugin-info.h>
#include <libpeas/peas-i18n.h>

#include "peas-gtk-plugin-manager.h"
#include "peas-gtk-plugin-manager-view.h"
#include "peas-gtk-configurable.h"

/**
 * SECTION:peas-gtk-plugin-manager
 * @short_description: Management GUI for plugins.
 *
 * The #PeasGtkPluginManager is a widget that can be used to manage plugins,
 * i.e. load or unload them, and see some pieces of information.
 *
 * <inlinegraphic fileref="peas-gtk-plugin-manager.png" format="PNG" />
 *
 * The only thing you need to do as an application writer if you wish to use
 * the manager to configure your plugins is to instantiate it using
 * peas_gtk_plugin_manager_new() and pack it into another widget or a window
 * (as in the screenshot above).
 *
 **/

struct _PeasGtkPluginManagerPrivate {
  PeasEngine *engine;

  GtkWidget *sw;
  GtkWidget *view;

  GtkWidget *about;

  GtkWidget *about_button;
  GtkWidget *configure_button;
};

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE,
  PROP_VIEW,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE (PeasGtkPluginManager,
                            peas_gtk_plugin_manager,
                            GTK_TYPE_BOX)

static GtkWindow *
get_toplevel (GtkWidget *widget)
{
  GtkWindow *toplevel;

  toplevel = (GtkWindow *) gtk_widget_get_toplevel (widget);
  if (!GTK_IS_WINDOW (toplevel))
    return NULL;

  /* Make sure the window always has a window group */
  if (!gtk_window_has_group (toplevel))
    {
      GtkWindowGroup *window_group;

      window_group = gtk_window_group_new ();
      gtk_window_group_add_window (window_group, toplevel);
      g_object_unref (window_group);
    }

  return toplevel;
}

static gboolean
plugin_is_configurable (PeasGtkPluginManager *pm,
                        PeasPluginInfo       *info)
{
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);

  if (info == NULL || !peas_plugin_info_is_loaded (info))
    return FALSE;

  return peas_engine_provides_extension (priv->engine, info,
                                         PEAS_GTK_TYPE_CONFIGURABLE);
}

static void
update_button_sensitivity (PeasGtkPluginManager *pm,
                           PeasPluginInfo       *info)
{
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);

  gtk_widget_set_sensitive (priv->about_button, info != NULL);
  gtk_widget_set_sensitive (priv->configure_button,
                            plugin_is_configurable (pm, info));
}

static void
show_about_cb (GtkWidget            *widget,
               PeasGtkPluginManager *pm)
{
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);
  PeasGtkPluginManagerView *view;
  PeasPluginInfo *info;
  GtkWindow *toplevel;
  gboolean modal;

  view = PEAS_GTK_PLUGIN_MANAGER_VIEW (priv->view);

  info = peas_gtk_plugin_manager_view_get_selected_plugin (view);
  g_return_if_fail (info != NULL);

  toplevel = get_toplevel (GTK_WIDGET (pm));
  modal = toplevel == NULL ? FALSE : gtk_window_get_modal (toplevel);

  /* If there is another about dialog already open destroy it */
  g_clear_pointer (&priv->about, (GDestroyNotify) gtk_widget_destroy);

  priv->about = GTK_WIDGET (g_object_new (GTK_TYPE_ABOUT_DIALOG,
                                          "program-name", peas_plugin_info_get_name (info),
                                          "copyright", peas_plugin_info_get_copyright (info),
                                          "authors", peas_plugin_info_get_authors (info),
                                          "comments", peas_plugin_info_get_description (info),
                                          "website", peas_plugin_info_get_website (info),
                                          "logo-icon-name", peas_plugin_info_get_icon_name (info),
                                          "version", peas_plugin_info_get_version (info),
                                          "destroy-with-parent", TRUE,
                                          "transient-for", toplevel,
                                          "modal", modal,
                                          NULL));

  g_signal_connect (priv->about,
                    "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);
  g_signal_connect (priv->about,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->about);

  gtk_widget_show (priv->about);
}

static void
help_button_cb (GtkWidget      *button,
                PeasPluginInfo *info)
{
  const gchar *help_uri;
#ifndef OS_OSX
  GError *error = NULL;
  GtkWidget *error_dlg;
#endif

  g_return_if_fail (peas_plugin_info_get_help_uri (info) != NULL);

  help_uri = peas_plugin_info_get_help_uri (info);

#ifdef OS_OSX
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:help_uri]]];
#else

  gtk_show_uri (NULL, help_uri, GDK_CURRENT_TIME, &error);
  if (error == NULL)
    return;

  g_debug ("Failed to show help URI: '%s'", help_uri);

  error_dlg = gtk_message_dialog_new (get_toplevel (button),
                                      GTK_DIALOG_MODAL |
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_CLOSE,
                                      _("There was an error displaying the help."));

  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (error_dlg),
                                            "%s", error->message);

  g_signal_connect (error_dlg,
                    "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);

  gtk_widget_show_all (error_dlg);

  g_error_free (error);
#endif
}

static void
show_configure_cb (GtkWidget            *widget,
                   PeasGtkPluginManager *pm)
{
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);
  PeasGtkPluginManagerView *view;
  PeasPluginInfo *info;
  PeasExtension *exten;
  GtkWidget *conf_widget = NULL;
  GtkWidget *conf_dlg;
  GtkWidget *vbox;

  view = PEAS_GTK_PLUGIN_MANAGER_VIEW (priv->view);

  info = peas_gtk_plugin_manager_view_get_selected_plugin (view);
  g_return_if_fail (info != NULL);

  exten = peas_engine_create_extension (priv->engine, info, PEAS_GTK_TYPE_CONFIGURABLE, NULL);
  g_return_if_fail (PEAS_IS_EXTENSION (exten));

  conf_widget = peas_gtk_configurable_create_configure_widget (PEAS_GTK_CONFIGURABLE (exten));
  g_object_unref (exten);

  g_return_if_fail (GTK_IS_WIDGET (conf_widget));
  g_return_if_fail (!gtk_widget_is_toplevel (conf_widget));

  conf_dlg = gtk_dialog_new_with_buttons (peas_plugin_info_get_name (info),
                                          get_toplevel (GTK_WIDGET (pm)),
                                          GTK_DIALOG_MODAL |
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          _("_Close"),
                                          GTK_RESPONSE_CLOSE,
                                          NULL);

  vbox = gtk_dialog_get_content_area (GTK_DIALOG (conf_dlg));
  gtk_box_pack_start (GTK_BOX (vbox), conf_widget, TRUE, TRUE, 0);

  if (peas_plugin_info_get_help_uri (info) != NULL)
    {
      GtkWidget *help_button;

      help_button = gtk_dialog_add_button (GTK_DIALOG (conf_dlg),
                                           _("_Help"), GTK_RESPONSE_HELP);

      g_signal_connect (help_button,
                        "clicked",
                        G_CALLBACK (help_button_cb),
                        info);
    }

  gtk_widget_show_all (conf_dlg);

  g_signal_connect (conf_dlg,
                    "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);
}

static void
plugin_loaded_toggled_cb (PeasEngine           *engine,
                          PeasPluginInfo       *info,
                          PeasGtkPluginManager *pm)
{
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);
  PeasGtkPluginManagerView *view;
  PeasPluginInfo *selected;

  view = PEAS_GTK_PLUGIN_MANAGER_VIEW (priv->view);
  selected = peas_gtk_plugin_manager_view_get_selected_plugin (view);

  if (selected == info)
    update_button_sensitivity (pm, info);
}

static void
selection_changed_cb (PeasGtkPluginManager *pm)
{
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);
  PeasGtkPluginManagerView *view;
  PeasPluginInfo *info;

  view = PEAS_GTK_PLUGIN_MANAGER_VIEW (priv->view);
  info = peas_gtk_plugin_manager_view_get_selected_plugin (view);

  update_button_sensitivity (pm, info);
}

static void
populate_popup_cb (PeasGtkPluginManagerView *view,
                   GtkMenu                  *menu,
                   PeasGtkPluginManager     *pm)
{
  PeasPluginInfo *info;
  GtkWidget *item;

  info = peas_gtk_plugin_manager_view_get_selected_plugin (view);

  if (info == NULL)
    return;

  item = gtk_check_menu_item_new_with_mnemonic (_("Pr_eferences"));
  g_signal_connect (item, "activate", G_CALLBACK (show_configure_cb), pm);
  gtk_widget_set_sensitive (item, plugin_is_configurable (pm, info));
  gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);

  item = gtk_check_menu_item_new_with_mnemonic (_("_About"));
  g_signal_connect (item, "activate", G_CALLBACK (show_about_cb), pm);
  gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
}

static void
peas_gtk_plugin_manager_init (PeasGtkPluginManager *pm)
{
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);
  GtkWidget *toolbar;
  GtkStyleContext *context;
  GtkToolItem *toolitem;
  GtkWidget *toolbar_box;
  GtkWidget *item_box;

  /* If we are using a PeasGtkPluginManager, we know for sure we will be using
     libpeas-gtk, so let's load the typelib for it here. */
  g_irepository_require (g_irepository_get_default (),
                         "PeasGtk", "1.0", 0, NULL);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (pm),
                                  GTK_ORIENTATION_VERTICAL);

  priv->sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->sw),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (priv->sw),
                                       GTK_SHADOW_IN);
  context = gtk_widget_get_style_context (priv->sw);
  gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
  gtk_box_pack_start (GTK_BOX (pm), priv->sw, TRUE, TRUE, 0);

  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_MENU);
  context = gtk_widget_get_style_context (toolbar);
  gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_INLINE_TOOLBAR);
  gtk_box_pack_start (GTK_BOX (pm), toolbar, FALSE, FALSE, 0);

  toolitem = gtk_tool_item_new ();
  gtk_tool_item_set_expand (toolitem, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, -1);

  /* this box is needed to get the items at the end of the toolbar */
  toolbar_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_container_add (GTK_CONTAINER (toolitem), toolbar_box);

  /* we need another box to disable css grouping */
  item_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_end (GTK_BOX (toolbar_box), item_box, FALSE, FALSE, 0);

  priv->about_button = gtk_button_new_with_mnemonic (_("_About"));
  gtk_box_pack_start (GTK_BOX (item_box), priv->about_button,
                      FALSE, FALSE, 0);

  /* we need another box to disable css grouping */
  item_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_end (GTK_BOX (toolbar_box), item_box, FALSE, FALSE, 0);

  priv->configure_button = gtk_button_new_with_mnemonic (_("_Preferences"));
  gtk_box_pack_start (GTK_BOX (item_box), priv->configure_button,
                      FALSE, FALSE, 0);

  /* setup a window of a sane size. */
  gtk_widget_set_size_request (GTK_WIDGET (priv->sw), 270, 100);

  g_signal_connect (priv->about_button,
                    "clicked",
                    G_CALLBACK (show_about_cb),
                    pm);
  g_signal_connect (priv->configure_button,
                    "clicked",
                    G_CALLBACK (show_configure_cb),
                    pm);
}

static void
peas_gtk_plugin_manager_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PeasGtkPluginManager *pm = PEAS_GTK_PLUGIN_MANAGER (object);
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);

  switch (prop_id)
    {
    case PROP_ENGINE:
      priv->engine = g_value_get_object (value);
      break;
    case PROP_VIEW:
      priv->view = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_plugin_manager_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PeasGtkPluginManager *pm = PEAS_GTK_PLUGIN_MANAGER (object);
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, priv->engine);
      break;
    case PROP_VIEW:
      g_value_set_object (value, peas_gtk_plugin_manager_get_view (pm));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_plugin_manager_constructed (GObject *object)
{
  PeasGtkPluginManager *pm = PEAS_GTK_PLUGIN_MANAGER (object);
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);
  GtkTreeSelection *selection;

  if (priv->engine == NULL)
    priv->engine = peas_engine_get_default ();

  g_object_ref (priv->engine);

  /* When we create the manager, we always rescan the plugins directory
   * Must come after the view has connected to notify::plugin-list
   */
  peas_engine_rescan_plugins (priv->engine);

  /* For the view to behave as expected, we must ensure it uses the same
   * engine as the manager itself.
   */
  if (priv->view != NULL)
    {
      PeasEngine *engine;

      g_object_get (priv->view,
                    "engine", &engine,
                    NULL);

      g_warn_if_fail (engine == priv->engine);

      if (engine != priv->engine)
        g_clear_object (&priv->view);

      g_object_unref (engine);
    }

  if (priv->view == NULL)
    priv->view = peas_gtk_plugin_manager_view_new (priv->engine);

  gtk_container_add (GTK_CONTAINER (priv->sw), priv->view);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->view));

  g_signal_connect_object (selection,
                           "changed",
                           G_CALLBACK (selection_changed_cb),
                           pm,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->view,
                           "cursor-changed",
                           G_CALLBACK (selection_changed_cb),
                           pm,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->view,
                           "populate-popup",
                           G_CALLBACK (populate_popup_cb),
                           pm,
                           0);
  g_signal_connect_object (priv->engine,
                           "load-plugin",
                           G_CALLBACK (plugin_loaded_toggled_cb),
                           pm,
                           G_CONNECT_AFTER);
  g_signal_connect_object (priv->engine,
                           "unload-plugin",
                           G_CALLBACK (plugin_loaded_toggled_cb),
                           pm,
                           G_CONNECT_AFTER);

  /* Update the button sensitivity */
  selection_changed_cb (pm);

  G_OBJECT_CLASS (peas_gtk_plugin_manager_parent_class)->constructed (object);
}

static void
peas_gtk_plugin_manager_dispose (GObject *object)
{
  PeasGtkPluginManager *pm = PEAS_GTK_PLUGIN_MANAGER (object);
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);

  g_clear_object (&priv->engine);
  g_clear_pointer (&priv->about, (GDestroyNotify) gtk_widget_destroy);

  G_OBJECT_CLASS (peas_gtk_plugin_manager_parent_class)->dispose (object);
}

static void
peas_gtk_plugin_manager_class_init (PeasGtkPluginManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_gtk_plugin_manager_set_property;
  object_class->get_property = peas_gtk_plugin_manager_get_property;
  object_class->constructed = peas_gtk_plugin_manager_constructed;
  object_class->dispose = peas_gtk_plugin_manager_dispose;

  /**
   * PeasGtkPluginManager:engine:
   *
   * The #PeasEngine this manager is attached to.
   */
  properties[PROP_ENGINE] =
    g_param_spec_object ("engine",
                         "engine",
                         "The PeasEngine this manager is attached to",
                         PEAS_TYPE_ENGINE,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * PeasGtkPluginManager:view:
   *
   * The #PeasGtkPluginManagerView shown in the #PeasGtkPluginManager.
   */
  properties[PROP_VIEW] =
    g_param_spec_object ("view",
                         "view",
                         "The view shown in the manager",
                         PEAS_GTK_TYPE_PLUGIN_MANAGER_VIEW,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

/**
 * peas_gtk_plugin_manager_new:
 * @engine: (allow-none): A #PeasEngine, or %NULL.
 *
 * Creates a new plugin manager for the given #PeasEngine.
 *
 * If @engine is %NULL, then the default engine will be used.
 *
 * Returns: the new #PeasGtkPluginManager.
 */
GtkWidget *
peas_gtk_plugin_manager_new (PeasEngine *engine)
{
  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);

  return GTK_WIDGET (g_object_new (PEAS_GTK_TYPE_PLUGIN_MANAGER,
                                   "engine", engine,
                                   NULL));
}

/**
 * peas_gtk_plugin_manager_get_view:
 * @pm: A #PeasGtkPluginManager.
 *
 * Returns the #PeasGtkPluginManagerView of @pm.
 *
 * Returns: (transfer none): the #GtkTreeView of @pm.
 */
GtkWidget *
peas_gtk_plugin_manager_get_view (PeasGtkPluginManager *pm)
{
  PeasGtkPluginManagerPrivate *priv = peas_gtk_plugin_manager_get_instance_private (pm);

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_MANAGER (pm), NULL);

  return priv->view;
}
