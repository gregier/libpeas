/*
 * gpe-plugin-info.c
 * This file is part of libgpe
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2007 - Steve Frécinaux
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <glib.h>

#include "gpe-plugin-info-priv.h"
#include "gpe-plugin.h"

void
_gpe_plugin_info_ref (GPEPluginInfo *info)
{
	g_atomic_int_inc (&info->refcount);
}

static GPEPluginInfo *
gpe_plugin_info_copy (GPEPluginInfo *info)
{
	_gpe_plugin_info_ref (info);
	return info;
}

void
_gpe_plugin_info_unref (GPEPluginInfo *info)
{
	if (!g_atomic_int_dec_and_test (&info->refcount))
		return;

	if (info->plugin != NULL)
		g_object_unref (info->plugin);

	g_free (info->file);
	g_free (info->module_dir);
	g_free (info->data_dir);
	g_free (info->module_name);
	g_strfreev (info->dependencies);
	g_free (info->name);
	g_free (info->desc);
	g_free (info->icon_name);
	g_free (info->website);
	g_free (info->copyright);
	g_free (info->loader);
	g_free (info->version);
	g_strfreev (info->authors);

	g_free (info);
}

/**
 * gpe_plugin_info_get_type:
 *
 * Retrieves the #GType object which is associated with the #GPEPluginInfo
 * class.
 *
 * Return value: the GType associated with #GPEPluginInfo.
 **/
GType
gpe_plugin_info_get_type (void)
{
	static GType the_type = 0;

	if (G_UNLIKELY (!the_type))
		the_type = g_boxed_type_register_static (
					"GPEPluginInfo",
					(GBoxedCopyFunc) gpe_plugin_info_copy,
					(GBoxedFreeFunc) _gpe_plugin_info_unref);

	return the_type;
}

/**
 * gpe_plugin_info_new:
 * @filename: the filename where to read the plugin information
 * @pathinfo: a #GPEPathInfo.
 *
 * Creates a new #GPEPluginInfo from a file on the disk.
 *
 * Return value: a newly created #GPEPluginInfo.
 */
GPEPluginInfo *
_gpe_plugin_info_new (const gchar       *file,
		      const GPEPathInfo *pathinfo)
{
	GPEPluginInfo *info;
	GKeyFile *plugin_file = NULL;
	gchar *str;

	g_return_val_if_fail (file != NULL, NULL);

	info = g_new0 (GPEPluginInfo, 1);
	info->refcount = 1;
	info->file = g_strdup (file);

	plugin_file = g_key_file_new ();
	if (!g_key_file_load_from_file (plugin_file, file, G_KEY_FILE_NONE, NULL))
	{
		g_warning ("Bad plugin file: %s", file);
		goto error;
	}

	if (!g_key_file_has_key (plugin_file, "Gedit Plugin", "IAge", NULL))
		goto error;

	/* Check IAge=2 */
	if (g_key_file_get_integer (plugin_file, "Gedit Plugin", "IAge", NULL) != 2)
		goto error;

	/* Get module name */
	str = g_key_file_get_string (plugin_file,
				     "Gedit Plugin",
				     "Module",
				     NULL);

	if ((str != NULL) && (*str != '\0'))
	{
		info->module_name = str;
	}
	else
	{
		g_warning ("Could not find 'Module' in %s", file);
		goto error;
	}

	/* Get the dependency list */
	info->dependencies = g_key_file_get_string_list (plugin_file,
							 "Gedit Plugin",
							 "Depends",
							 NULL,
							 NULL);
	if (info->dependencies == NULL)
		info->dependencies = g_new0 (gchar *, 1);

	/* Get the loader for this plugin */
	str = g_key_file_get_string (plugin_file,
				     "Gedit Plugin",
				     "Loader",
				     NULL);

	if ((str != NULL) && (*str != '\0'))
	{
		info->loader = str;
	}
	else
	{
		/* default to the C loader */
		info->loader = g_strdup("c");
	}

	/* Get Name */
	str = g_key_file_get_locale_string (plugin_file,
					    "Gedit Plugin",
					    "Name",
					    NULL, NULL);
	if (str)
		info->name = str;
	else
	{
		g_warning ("Could not find 'Name' in %s", file);
		goto error;
	}

	/* Get Description */
	str = g_key_file_get_locale_string (plugin_file,
					    "Gedit Plugin",
					    "Description",
					    NULL, NULL);
	if (str)
		info->desc = str;

	/* Get Icon */
	str = g_key_file_get_locale_string (plugin_file,
					    "Gedit Plugin",
					    "Icon",
					    NULL, NULL);
	if (str)
		info->icon_name = str;

	/* Get Authors */
	info->authors = g_key_file_get_string_list (plugin_file,
						    "Gedit Plugin",
						    "Authors",
						    NULL,
						    NULL);

	/* Get Copyright */
	str = g_key_file_get_string (plugin_file,
				     "Gedit Plugin",
				     "Copyright",
				     NULL);
	if (str)
		info->copyright = str;

	/* Get Website */
	str = g_key_file_get_string (plugin_file,
				     "Gedit Plugin",
				     "Website",
				     NULL);
	if (str)
		info->website = str;

	/* Get Version */
	str = g_key_file_get_string (plugin_file,
				     "Gedit Plugin",
				     "Version",
				     NULL);
	if (str)
		info->version = str;

	g_key_file_free (plugin_file);

	info->module_dir = g_strdup (pathinfo->module_dir);
	info->data_dir = g_build_filename (pathinfo->data_dir, info->module_name, NULL);

	/* If we know nothing about the availability of the plugin,
	   set it as available */
	info->available = TRUE;

	return info;

error:
	g_free (info->file);
	g_free (info->module_name);
	g_free (info->name);
	g_free (info->loader);
	g_free (info);
	g_key_file_free (plugin_file);

	return NULL;
}

gboolean
gpe_plugin_info_is_active (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	return info->available && info->plugin != NULL;
}

gboolean
gpe_plugin_info_is_available (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	return info->available != FALSE;
}

gboolean
gpe_plugin_info_is_configurable (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	if (info->plugin == NULL || !info->available)
		return FALSE;

	return gpe_plugin_is_configurable (info->plugin);
}

const gchar *
gpe_plugin_info_get_module_name (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->module_name;
}

const gchar *
gpe_plugin_info_get_module_dir (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->module_dir;
}

const gchar *
gpe_plugin_info_get_data_dir (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->data_dir;
}

const gchar *
gpe_plugin_info_get_name (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->name;
}

const gchar *
gpe_plugin_info_get_description (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->desc;
}

const gchar *
gpe_plugin_info_get_icon_name (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	/* use the gedit-plugin icon as a default if the plugin does not
	   have its own */
	if (info->icon_name != NULL &&
	    gtk_icon_theme_has_icon (gtk_icon_theme_get_default (),
				     info->icon_name))
		return info->icon_name;
	else
		return "gedit-plugin";
}

const gchar **
gpe_plugin_info_get_authors (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, (const gchar **)NULL);

	return (const gchar **) info->authors;
}

const gchar *
gpe_plugin_info_get_website (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->website;
}

const gchar *
gpe_plugin_info_get_copyright (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->copyright;
}

const gchar *
gpe_plugin_info_get_version (GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->version;
}
