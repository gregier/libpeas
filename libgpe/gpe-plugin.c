/*
 * gpe-plugin.h
 * This file is part of libgpe
 *
 * Copyright (C) 2002-2005 Paolo Maggi
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

#include "gpe-plugin.h"
#include "gpe-dirs.h"

/* properties */
enum {
	PROP_0,
	PROP_INSTALL_DIR,
	PROP_DATA_DIR
};

typedef struct _GPEPluginPrivate GPEPluginPrivate;

struct _GPEPluginPrivate
{
	gchar *install_dir;
	gchar *data_dir;
};

#define GPE_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GPE_TYPE_PLUGIN, GPEPluginPrivate))

G_DEFINE_TYPE(GPEPlugin, gpe_plugin, G_TYPE_OBJECT)

static void
dummy (GPEPlugin *plugin, GObject *target_object)
{
	/* Empty */
}

static GtkWidget *
create_configure_dialog	(GPEPlugin *plugin)
{
	return NULL;
}

static gboolean
is_configurable (GPEPlugin *plugin)
{
	return (GPE_PLUGIN_GET_CLASS (plugin)->create_configure_dialog !=
		create_configure_dialog);
}

static void
gpe_plugin_get_property (GObject    *object,
				 guint       prop_id,
				 GValue     *value,
				 GParamSpec *pspec)
{
	switch (prop_id)
	{
		case PROP_INSTALL_DIR:
			g_value_take_string (value, gpe_plugin_get_install_dir (GPE_PLUGIN (object)));
			break;
		case PROP_DATA_DIR:
			g_value_take_string (value, gpe_plugin_get_data_dir (GPE_PLUGIN (object)));
			break;
		default:
			g_return_if_reached ();
	}
}

static void
gpe_plugin_set_property (GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
	GPEPluginPrivate *priv = GPE_PLUGIN_GET_PRIVATE (object);

	switch (prop_id)
	{
		case PROP_INSTALL_DIR:
			priv->install_dir = g_value_dup_string (value);
			break;
		case PROP_DATA_DIR:
			priv->data_dir = g_value_dup_string (value);
			break;
		default:
			g_return_if_reached ();
	}
}

static void
gpe_plugin_finalize (GObject *object)
{
	GPEPluginPrivate *priv = GPE_PLUGIN_GET_PRIVATE (object);

	g_free (priv->install_dir);
	g_free (priv->data_dir);

	G_OBJECT_CLASS (gpe_plugin_parent_class)->finalize (object);
}

static void
gpe_plugin_class_init (GPEPluginClass *klass)
{
    	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	klass->activate = dummy;
	klass->deactivate = dummy;
	klass->update_ui = dummy;

	klass->create_configure_dialog = create_configure_dialog;
	klass->is_configurable = is_configurable;

	object_class->get_property = gpe_plugin_get_property;
	object_class->set_property = gpe_plugin_set_property;
	object_class->finalize = gpe_plugin_finalize;

	g_object_class_install_property (object_class,
					 PROP_INSTALL_DIR,
					 g_param_spec_string ("install-dir",
							      "Install Directory",
							      "The directory where the plugin is installed",
							      NULL,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_DATA_DIR,
					 g_param_spec_string ("data-dir",
							      "Data Directory",
							      "The full path of the directory where the plugin should look for its data files",
							      NULL,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (klass, sizeof (GPEPluginPrivate));
}

static void
gpe_plugin_init (GPEPlugin *plugin)
{
	/* Empty */
}

/**
 * gpe_plugin_get_install_dir:
 * @plugin: a #GPEPlugin
 *
 * Get the path of the directory where the plugin is installed.
 *
 * Return value: a newly allocated string with the path of the
 * directory where the plugin is installed
 */
gchar *
gpe_plugin_get_install_dir (GPEPlugin *plugin)
{
	g_return_val_if_fail (GPE_IS_PLUGIN (plugin), NULL);

	return g_strdup (GPE_PLUGIN_GET_PRIVATE (plugin)->install_dir);
}

/**
 * gpe_plugin_get_data_dir:
 * @plugin: a #GPEPlugin
 *
 * Get the path of the directory where the plugin should look for
 * its data files.
 *
 * Return value: a newly allocated string with the path of the
 * directory where the plugin should look for its data files
 */
gchar *
gpe_plugin_get_data_dir (GPEPlugin *plugin)
{
	g_return_val_if_fail (GPE_IS_PLUGIN (plugin), NULL);

	return g_strdup (GPE_PLUGIN_GET_PRIVATE (plugin)->data_dir);
}

/**
 * gpe_plugin_activate:
 * @plugin: a #GPEPlugin
 * @target_object: a #GObject
 *
 * Activates the plugin.
 */
void
gpe_plugin_activate (GPEPlugin *plugin,
			     GObject          *target_object)
{
	g_return_if_fail (GPE_IS_PLUGIN (plugin));
	g_return_if_fail (G_IS_OBJECT (target_object));

	GPE_PLUGIN_GET_CLASS (plugin)->activate (plugin, target_object);
}

/**
 * gpe_plugin_deactivate:
 * @plugin: a #GPEPlugin
 * @target_object: a #GObject
 *
 * Deactivates the plugin.
 */
void
gpe_plugin_deactivate	(GPEPlugin *plugin,
				 GObject          *target_object)
{
	g_return_if_fail (GPE_IS_PLUGIN (plugin));
	g_return_if_fail (G_IS_OBJECT (target_object));

	GPE_PLUGIN_GET_CLASS (plugin)->deactivate (plugin, target_object);
}

/**
 * gpe_plugin_update_ui:
 * @plugin: a #GPEPlugin
 * @target_object: a #GObject
 *
 * Triggers an update of the user interface to take into account state changes
 * caused by the plugin.
 */
void
gpe_plugin_update_ui	(GPEPlugin *plugin,
				 GObject          *target_object)
{
	g_return_if_fail (GPE_IS_PLUGIN (plugin));
	g_return_if_fail (G_IS_OBJECT (target_object));

	GPE_PLUGIN_GET_CLASS (plugin)->update_ui (plugin, target_object);
}

/**
 * gpe_plugin_is_configurable:
 * @plugin: a #GPEPlugin
 *
 * Whether the plugin is configurable.
 *
 * Returns: TRUE if the plugin is configurable:
 */
gboolean
gpe_plugin_is_configurable (GPEPlugin *plugin)
{
	g_return_val_if_fail (GPE_IS_PLUGIN (plugin), FALSE);

	return GPE_PLUGIN_GET_CLASS (plugin)->is_configurable (plugin);
}

/**
 * gpe_plugin_create_configure_dialog:
 * @plugin: a #GPEPlugin
 *
 * Creates the configure dialog widget for the plugin.
 *
 * Returns: the configure dialog widget for the plugin.
 */
GtkWidget *
gpe_plugin_create_configure_dialog (GPEPlugin *plugin)
{
	g_return_val_if_fail (GPE_IS_PLUGIN (plugin), NULL);

	return GPE_PLUGIN_GET_CLASS (plugin)->create_configure_dialog (plugin);
}
