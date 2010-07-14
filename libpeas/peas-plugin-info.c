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

  if (info->keys != NULL)
    g_hash_table_destroy (info->keys);
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

/*
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
    the_type = g_boxed_type_register_static (g_intern_static_string ("PeasPluginInfo"),
                                             (GBoxedCopyFunc) _peas_plugin_info_ref,
                                             (GBoxedFreeFunc) _peas_plugin_info_unref);

  return the_type;
}

static void
value_free (GValue *value)
{
  g_value_unset (value);
  g_free (value);
}

static void
parse_extra_keys (PeasPluginInfo   *info,
                  GKeyFile         *plugin_file,
                  const gchar      *section_header,
                  const gchar     **keys)
{
  guint i;

  for (i = 0; keys[i] != NULL; i++)
    {
      GValue *value = NULL;
      gboolean b;
      GError *error = NULL;

      if (g_str_equal (keys[i], "IAge") ||
          g_str_equal (keys[i], "Module") ||
          g_str_equal (keys[i], "Depends") ||
          g_str_equal (keys[i], "Loader") ||
          g_str_equal (keys[i], "Name") ||
          g_str_has_prefix (keys[i], "Name[") ||
          g_str_equal (keys[i], "Description") ||
          g_str_has_prefix (keys[i], "Description[") ||
          g_str_equal (keys[i], "Icon") ||
          g_str_equal (keys[i], "Authors") ||
          g_str_equal (keys[i], "Copyright") ||
          g_str_equal (keys[i], "Website") ||
          g_str_equal (keys[i], "Version") ||
          g_str_equal (keys[i], "Builtin"))
        continue;

      b = g_key_file_get_boolean (plugin_file, section_header, keys[i], &error);
      if (b == FALSE && error != NULL)
        {
          gchar *str;
          g_error_free (error);
          error = NULL;
          str = g_key_file_get_string (plugin_file, section_header, keys[i], NULL);
          if (str != NULL)
            {
              value = g_new0 (GValue, 1);
              g_value_init (value, G_TYPE_STRING);
              g_value_take_string (value, str);
            }
        }
      else
        {
          value = g_new0 (GValue, 1);
          g_value_init (value, G_TYPE_BOOLEAN);
          g_value_set_boolean (value, b);
        }

      if (!value)
        continue;

      if (info->keys == NULL)
        {
          info->keys = g_hash_table_new_full (g_str_hash,
                                              g_str_equal,
                                              g_free,
                                              (GDestroyNotify) value_free);
        }
      g_hash_table_insert (info->keys, g_strdup (keys[i]), value);
    }
}

/*
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
  gchar **keys;
  gint integer;
  gboolean b;
  GError *error = NULL;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (app_name != NULL, NULL);

  info = g_new0 (PeasPluginInfo, 1);
  info->refcount = 1;
  info->file = g_strdup (filename);

  section_header = g_strdup_printf ("%s Plugin", app_name);

  plugin_file = g_key_file_new ();
  if (!g_key_file_load_from_file (plugin_file, filename, G_KEY_FILE_NONE, NULL))
    {
      g_warning ("Bad plugin file: '%s'", filename);
      goto error;
    }

  if (!g_key_file_has_key (plugin_file, section_header, "IAge", NULL))
    goto error;

  integer = g_key_file_get_integer (plugin_file, section_header, "IAge", NULL);
  info->iage = integer <= 0 ? 0 : integer;

  /* Get module name */
  str = g_key_file_get_string (plugin_file, section_header, "Module", NULL);

  if ((str != NULL) && (*str != '\0'))
    {
      info->module_name = str;
    }
  else
    {
      g_warning ("Could not find 'Module' in '%s'", filename);
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
      info->loader = g_strdup ("C");
    }

  /* Get Name */
  str = g_key_file_get_locale_string (plugin_file,
                                      section_header, "Name", NULL, NULL);
  if (str)
    info->name = str;
  else
    {
      g_warning ("Could not find 'Name' in '%s'", filename);
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

  /* Get Builtin */
  b = g_key_file_get_boolean (plugin_file, section_header, "Builtin", &error);
  if (error != NULL)
    g_clear_error (&error);
  else
    info->builtin = b;

  /* Get extra keys */
  keys = g_key_file_get_keys (plugin_file, section_header, NULL, NULL);
  parse_extra_keys (info, plugin_file, section_header, (const gchar **) keys);
  g_strfreev (keys);

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
 * peas_plugin_info_is_loaded:
 * @info: A #PeasPluginInfo.
 *
 * Check if the plugin is loaded.
 *
 * Returns: %TRUE if the plugin is loaded.
 */
gboolean
peas_plugin_info_is_loaded (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  return info->available && info->loaded;
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
peas_plugin_info_is_available (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  return info->available != FALSE;
}

/**
 * peas_plugin_info_is_builtin:
 * @info: A #PeasPluginInfo.
 *
 * Gets is the plugin is a builtin plugin.
 *
 * Returns: %TRUE if the plugin is a builtin plugin, %FALSE
 * if not.
 **/
gboolean
peas_plugin_info_is_builtin (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, TRUE);

  return info->builtin;
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
peas_plugin_info_get_module_name (const PeasPluginInfo *info)
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
peas_plugin_info_get_module_dir (const PeasPluginInfo *info)
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
peas_plugin_info_get_data_dir (const PeasPluginInfo *info)
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
peas_plugin_info_get_name (const PeasPluginInfo *info)
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
peas_plugin_info_get_description (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->desc;
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
peas_plugin_info_get_authors (const PeasPluginInfo *info)
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
peas_plugin_info_get_website (const PeasPluginInfo *info)
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
peas_plugin_info_get_copyright (const PeasPluginInfo *info)
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
peas_plugin_info_get_version (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->version;
}

/**
 * peas_plugin_info_get_iage:
 * @info: A #PeasPluginInfo.
 *
 * Gets the interface age of the plugin.
 *
 * Returns: the interface age of the plugin or %0 if not known.
 **/
gint
peas_plugin_info_get_iage (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, 0);

  return info->iage;
}

/**
 * peas_plugin_info_get_keys:
 * @info: A #PeasPluginInfo.
 *
 * Gets a hash table of string keys present and #GValue values,
 * present in the plugin information file, but not handled
 * by libpeas. Note that libpeas only handles booleans and
 * strings, and that strings that are recognised as booleans,
 * as done by #g_key_file_get_boolean, will be of boolean type.
 *
 * Returns: a #GHashTable of string keys and #GValue values. Do
 * not free or destroy any data in this hashtable.
 **/
const GHashTable *
peas_plugin_info_get_keys (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->keys;
}
