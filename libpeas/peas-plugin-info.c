/*
 * peas-plugin-info.c
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2007 - Steve Frécinaux
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
#include <glib.h>

#include "peas-i18n.h"
#include "peas-plugin-info-priv.h"
#include "peas-plugin.h"

/**
 * SECTION:peas-plugin-info
 * @short_description: Information about a plugin.
 *
 * A #PeasPluginInfo contains all the information available about a plugin.
 **/

PeasPluginInfo *
_peas_plugin_info_ref (PeasPluginInfo *info)
{
  g_atomic_int_inc (&info->refcount);
  return info;
}

void
_peas_plugin_info_unref (PeasPluginInfo *info)
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
 * peas_plugin_info_get_type:
 *
 * Retrieves the #GType object which is associated with the #PeasPluginInfo
 * class.
 *
 * Return value: the GType associated with #PeasPluginInfo.
 **/
GType
peas_plugin_info_get_type (void)
{
  static GType the_type = 0;

  if (G_UNLIKELY (!the_type))
    the_type = g_boxed_type_register_static ("PeasPluginInfo",
                                             (GBoxedCopyFunc) _peas_plugin_info_ref,
                                             (GBoxedFreeFunc) _peas_plugin_info_unref);

  return the_type;
}

/**
 * _peas_plugin_info_new:
 * @filename: The filename where to read the plugin information.
 * @app_name: The application name.
 * @module_dir: The module directory.
 * @data_dir: The data directory.
 *
 * Creates a new #PeasPluginInfo from a file on the disk.
 *
 * Return value: a newly created #PeasPluginInfo.
 */
PeasPluginInfo *
_peas_plugin_info_new (const gchar *filename,
                       const gchar *app_name,
                       const gchar *module_dir,
                       const gchar *data_dir)
{
  PeasPluginInfo *info;
  GKeyFile *plugin_file = NULL;
  gchar *section_header;
  gchar *str;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (app_name != NULL, NULL);

  info = g_new0 (PeasPluginInfo, 1);
  info->refcount = 1;
  info->file = g_strdup (filename);

  section_header = g_strdup_printf ("%s Plugin", app_name);

  plugin_file = g_key_file_new ();
  if (!g_key_file_load_from_file (plugin_file, filename, G_KEY_FILE_NONE, NULL))
    {
      g_warning ("Bad plugin file: %s", filename);
      goto error;
    }

  if (!g_key_file_has_key (plugin_file, section_header, "IAge", NULL))
    goto error;

  /* Check IAge=2 */
  if (g_key_file_get_integer (plugin_file, section_header, "IAge", NULL) != 2)
    goto error;

  /* Get module name */
  str = g_key_file_get_string (plugin_file, section_header, "Module", NULL);

  if ((str != NULL) && (*str != '\0'))
    {
      info->module_name = str;
    }
  else
    {
      g_warning ("Could not find 'Module' in %s", filename);
      goto error;
    }

  /* Get the dependency list */
  info->dependencies = g_key_file_get_string_list (plugin_file,
                                                   section_header,
                                                   "Depends", NULL, NULL);
  if (info->dependencies == NULL)
    info->dependencies = g_new0 (gchar *, 1);

  /* Get the loader for this plugin */
  str = g_key_file_get_string (plugin_file, section_header, "Loader", NULL);

  if ((str != NULL) && (*str != '\0'))
    {
      info->loader = str;
    }
  else
    {
      /* default to the C loader */
      info->loader = g_strdup ("c");
    }

  /* Get Name */
  str = g_key_file_get_locale_string (plugin_file,
                                      section_header, "Name", NULL, NULL);
  if (str)
    info->name = str;
  else
    {
      g_warning ("Could not find 'Name' in %s", filename);
      goto error;
    }

  /* Get Description */
  str = g_key_file_get_locale_string (plugin_file,
                                      section_header,
                                      "Description", NULL, NULL);
  if (str)
    info->desc = str;

  /* Get Icon */
  str = g_key_file_get_locale_string (plugin_file,
                                      section_header, "Icon", NULL, NULL);
  if (str)
    info->icon_name = str;

  /* Get Authors */
  info->authors = g_key_file_get_string_list (plugin_file,
                                              section_header,
                                              "Authors", NULL, NULL);

  /* Get Copyright */
  str = g_key_file_get_string (plugin_file,
                               section_header, "Copyright", NULL);
  if (str)
    info->copyright = str;

  /* Get Website */
  str = g_key_file_get_string (plugin_file, section_header, "Website", NULL);
  if (str)
    info->website = str;

  /* Get Version */
  str = g_key_file_get_string (plugin_file, section_header, "Version", NULL);
  if (str)
    info->version = str;

  g_free (section_header);
  g_key_file_free (plugin_file);

  info->module_dir = g_strdup (module_dir);
  info->data_dir = g_build_filename (data_dir, info->module_name, NULL);

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
  g_free (section_header);
  g_key_file_free (plugin_file);

  return NULL;
}

/**
 * peas_plugin_info_is_active:
 * @info: A #PeasPluginInfo.
 *
 * Check if the plugin is active.
 *
 * Returns: %TRUE if the plugin is active.
 */
gboolean
peas_plugin_info_is_active (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  return info->available && info->plugin != NULL;
}

/**
 * peas_plugin_info_is_available:
 * @info: A #PeasPluginInfo.
 *
 * Check if the plugin is available.  A plugin is marked as not available when
 * there is no loader available to load it, or when there has been an error
 * when trying to load it previously.
 *
 * Returns: %TRUE if the plugin is available.
 */
gboolean
peas_plugin_info_is_available (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  return info->available != FALSE;
}

/**
 * peas_plugin_info_is_configurable:
 * @info: A #PeasPluginInfo.
 *
 * Check if the plugin is configurable.
 *
 * Returns: %TRUE if the plugin is configurable.
 */
gboolean
peas_plugin_info_is_configurable (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  if (info->plugin == NULL || !info->available)
    return FALSE;

  return peas_plugin_is_configurable (info->plugin);
}

/**
 * peas_plugin_info_get_module_name:
 * @info: A #PeasPluginInfo.
 *
 * Gets the module name.
 *
 * Returns: the module name.
 */
const gchar *
peas_plugin_info_get_module_name (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->module_name;
}

/**
 * peas_plugin_info_get_module_dir:
 * @info: A #PeasPluginInfo.
 *
 * Gets the module directory.
 *
 * Returns: the module directory.
 */
const gchar *
peas_plugin_info_get_module_dir (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->module_dir;
}

/**
 * peas_plugin_info_get_data_dir:
 * @info: A #PeasPluginInfo.
 *
 * Gets the data dir of the plugin.
 *
 * Returns: the plugin's data dir.
 */
const gchar *
peas_plugin_info_get_data_dir (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->data_dir;
}

/**
 * peas_plugin_info_get_name:
 * @info: A #PeasPluginInfo.
 *
 * Gets the name of the plugin.
 *
 * Returns: the plugin's name.
 */
const gchar *
peas_plugin_info_get_name (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->name;
}

/**
 * peas_plugin_info_get_description:
 * @info: A #PeasPluginInfo.
 *
 * Gets the description of the plugin.
 *
 * Returns: the plugin's description.
 */
const gchar *
peas_plugin_info_get_description (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->desc;
}

/**
 * peas_plugin_info_get_icon_name:
 * @info: A #PeasPluginInfo.
 *
 * Gets the icon name of the plugin.
 *
 * Returns: the plugin's icon name.
 */
const gchar *
peas_plugin_info_get_icon_name (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  /* use the libpeas-plugin icon as a default if the plugin does not
     have its own */
  if (info->icon_name != NULL &&
      gtk_icon_theme_has_icon (gtk_icon_theme_get_default (),
                               info->icon_name))
    return info->icon_name;
  else
    return "libpeas-plugin";
}

/**
 * peas_plugin_info_get_authors:
 * @info: A #PeasPluginInfo.
 *
 * Gets a NULL-terminated array of strings with the authors of the plugin.
 *
 * Returns: the plugin's author list.
 */
const gchar **
peas_plugin_info_get_authors (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, (const gchar **) NULL);

  return (const gchar **) info->authors;
}

/**
 * peas_plugin_info_get_website:
 * @info: A #PeasPluginInfo.
 *
 * Gets the website of the plugin.
 *
 * Returns: the plugin's associated website.
 */
const gchar *
peas_plugin_info_get_website (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->website;
}

/**
 * peas_plugin_info_get_copyright:
 * @info: A #PeasPluginInfo.
 *
 * Gets the copyright of the plugin.
 *
 * Returns: the plugin's copyright information.
 */
const gchar *
peas_plugin_info_get_copyright (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->copyright;
}

/**
 * peas_plugin_info_get_version:
 * @info: A #PeasPluginInfo.
 *
 * Gets the version of the plugin.
 *
 * Returns: the plugin's version.
 */
const gchar *
peas_plugin_info_get_version (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->version;
}
