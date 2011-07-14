/*
 * plugin-infos-to-xml.c
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

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gio/gio.h>
#include <libxml/xmlwriter.h>

#define PLUGIN_STORE_VERSION "1.0"

/* Wrap common xml calls to error out if they fail */
#define xmlTextWriterStartElement(w, name) \
  if (xmlTextWriterStartElement (w, name) < 0) \
    g_error ("Could not write '%s' element to xml document", name)

#define xmlTextWriterEndElement(w) \
  if (xmlTextWriterEndElement (w) < 0) \
    g_error ("Could not end element in xml document")

#define xmlTextWriterWriteAttribute(w, name, value) \
  if (xmlTextWriterWriteAttribute (w, name, value) < 0) \
    g_error ("Could not write '%s' attribute to xml document", name)

#define xmlTextWriterWriteElement(w, name, content) \
  if (xmlTextWriterWriteElement (w, name, content) < 0) \
    g_error ("Could not write '%s' element to xml document", name)

#define xmlTextWriterWriteString(w, content) \
  if (xmlTextWriterWriteString (w, content) < 0) \
    g_error ("Could not write string to xml document")

gchar *opt_app;
gchar *opt_data_dir;
gchar *opt_module_dir;
gchar *opt_output_dir;
gchar **opt_files;

GFile *output_location;
GFile *plugins_location;
GFile *icons_location;

static void show_version (void);

static GOptionEntry entries[] =
{
  { "application",        0, 0, G_OPTION_ARG_STRING,         &opt_app,        "Application ID"    },
  { "data-dir",           0, 0, G_OPTION_ARG_STRING,         &opt_data_dir,   "Data Directory"    },
  { "module-dir",         0, 0, G_OPTION_ARG_STRING,         &opt_module_dir, "Module Directory"  },
  { "output-dir",         0, 0, G_OPTION_ARG_FILENAME,       &opt_output_dir, "Output Directory", },
  { "version",          'v', 0, G_OPTION_ARG_CALLBACK,       show_version,    "Show Version"      },
  { G_OPTION_REMAINING,   0, 0, G_OPTION_ARG_FILENAME_ARRAY, &opt_files,      "Plugin Info Files" },
  { NULL }
};

static void
show_version (void)
{
  g_print ("%s - Version 0.0.1\n", g_get_prgname ());
  exit (0);
}

static gchar *
get_lang (const gchar *key)
{
  gchar *locale;

  locale = g_strrstr (key, "[");

  if (locale != NULL && strlen (locale) <= 2)
    locale = NULL;

  if (locale != NULL)
    locale = g_strndup (locale + 1, strlen (locale) - 2);

  return locale;
}

static void
process_plugin (const gchar      *filename,
                xmlTextWriterPtr  writer)
{
  gint i;
  GKeyFile *plugin_file;
  gchar *module_name;
  gchar *loader;
  gchar **keys;
  GFile *plugin_location;

  if (!g_str_has_suffix (filename, ".plugin"))
    {
      g_printerr ("Error: plugin info files must have .plugin extension.\n");
      exit (1);
    }

  plugin_file = g_key_file_new ();

  if (!g_key_file_load_from_file (plugin_file, filename,
                                  G_KEY_FILE_KEEP_TRANSLATIONS, NULL))
    {
      g_printerr ("Error: failed to load file '%s'.\n", filename);
      exit (1);
    }

  if (!g_key_file_has_group (plugin_file, "Plugin"))
    {
      g_printerr ("Error: '%s' does not have 'Plugin' group.\n", filename);
      exit (1);
    }

  module_name = g_key_file_get_string (plugin_file, "Plugin", "Module", NULL);

  if (module_name == NULL)
    {
      g_printerr ("Error: '%s' does not have a 'Module' key.\n", filename);
      exit (1);
    }

  xmlTextWriterWriteFormatComment (writer, " %s ", filename);

  xmlTextWriterStartElement (writer, BAD_CAST "plugin");
  xmlTextWriterSetIndent (writer, 3);

  loader = g_key_file_get_string (plugin_file, "Plugin", "Loader", NULL);

  /* Skip C plugins */
  if (loader == NULL || g_ascii_strcasecmp (loader, "C") == 0)
    {
      if (FALSE)
        goto out;

      xmlTextWriterWriteElement (writer, BAD_CAST "loader", BAD_CAST "C");
    }

  keys = g_key_file_get_keys (plugin_file, "Plugin", NULL, NULL);

  for (i = 0; keys[i] != NULL; ++i)
    {
      gchar *el_name;
      gchar *str;
      gchar *lang;

      if (g_strcmp0 (keys[i], "IAge") == 0)
        continue;

      el_name = g_ascii_strdown (keys[i], -1);
      str = g_key_file_get_string (plugin_file, "Plugin", keys[i], NULL);

      if (g_str_has_prefix (keys[i], "Name["))
        {
          lang = get_lang (keys[i]);
          xmlTextWriterStartElement (writer, BAD_CAST el_name);
          xmlTextWriterWriteAttribute (writer, BAD_CAST "xml:lang", BAD_CAST lang);
          xmlTextWriterWriteString (writer, BAD_CAST str);
          xmlTextWriterEndElement (writer);
          g_free (lang);
        }
      else if (g_str_has_prefix (keys[i], "Description["))
        {
          lang = get_lang (keys[i]);
          xmlTextWriterStartElement (writer, BAD_CAST el_name);
          xmlTextWriterWriteAttribute (writer, BAD_CAST "xml:lang", BAD_CAST lang);
          xmlTextWriterWriteString (writer, BAD_CAST str);
          xmlTextWriterEndElement (writer);
          g_free (lang);
        }
      else if (g_strcmp0 (keys[i], "Icon") == 0)
        {
          GFile *plugin_location;
          GFile *plugin_dir_location;
          GFile *icon_location;

          plugin_location = g_file_new_for_commandline_arg (filename);
          plugin_dir_location = g_file_get_parent (plugin_location);
          icon_location = g_file_get_child (plugin_dir_location, str);

          if (!g_file_query_exists (icon_location, NULL))
            {
              xmlTextWriterWriteElement (writer, BAD_CAST el_name,
                                         BAD_CAST str);
            }
          else
            {
              GFile *output_icon_location;

              output_icon_location = g_file_get_child (icons_location,
                                                       module_name);

              if (output_icon_location == NULL ||
                  !g_file_copy (icon_location, output_icon_location,
                                G_FILE_COPY_NONE, NULL, NULL, NULL, NULL))
                {
                  g_printerr ("Error: failed to copy '%s' to '%s'.\n",
                              g_file_get_path (icon_location),
                              g_file_get_path (output_icon_location));
                  exit (1);
                }

              xmlTextWriterWriteElement (writer, BAD_CAST el_name,
                                         BAD_CAST module_name);

              g_object_unref (output_icon_location);
            }

          g_object_unref (icon_location);
          g_object_unref (plugin_dir_location);
          g_object_unref (plugin_location);
        }
      else if (g_strcmp0 (keys[i], "Module") == 0 ||
               g_strcmp0 (keys[i], "Loader") == 0 ||
               g_strcmp0 (keys[i], "Name") == 0 ||
               g_strcmp0 (keys[i], "Description") == 0 ||
               g_strcmp0 (keys[i], "Depends") == 0 ||
               g_strcmp0 (keys[i], "Authors") == 0 ||
               g_strcmp0 (keys[i], "Website") == 0 ||
               g_strcmp0 (keys[i], "Copyright") == 0 ||
               g_strcmp0 (keys[i], "Version") == 0 ||
               g_strcmp0 (keys[i], "Help") == 0 ||
               g_strcmp0 (keys[i], "Help-Windows") == 0 ||
               g_strcmp0 (keys[i], "Help-MacOS-X") == 0 ||
               g_strcmp0 (keys[i], "Help-GNOME") == 0)
        {
          xmlTextWriterWriteElement (writer, BAD_CAST el_name, BAD_CAST str);
        }
      else
        {
          g_printerr ("Error: invalid key '%s'.\n", keys[i]);
          exit (1);
        }

      g_free (el_name);
      g_free (str);
    }

  g_strfreev (keys);

  xmlTextWriterEndElement (writer);
  xmlTextWriterSetIndent (writer, 2);

  plugin_location = g_file_get_child (plugins_location, module_name);

  g_object_unref (g_file_create (plugin_location, G_FILE_CREATE_NONE,
                                 NULL, NULL));

  g_object_unref (plugin_location);

out:

  g_free (module_name);
  g_free (loader);
  g_key_file_free (plugin_file);
}

static void
create_dir (GFile *location)
{
  if (g_file_query_exists (location, NULL))
    {
      g_printerr ("Error: the output dir must not exist yet.\n");
      exit (1);
    }

  if (!g_file_make_directory_with_parents (location, NULL, NULL))
    {
      g_printerr ("Error: could not create '%s'\n",
                  g_file_get_path (location));
      exit (1);
    }
}

static void
to_app_location (GFile **location)
{
  GFile *new_location;

  new_location = g_file_get_child (*location, opt_app);

  g_object_unref (*location);

  *location = new_location;
}

int
main (int    argc,
      char **argv)
{
  int i;
  GError *error = NULL;
  GOptionContext *context;
  gchar *filename_tmp;
  gchar *filename;
  xmlTextWriterPtr writer;

  LIBXML_TEST_VERSION

  context = g_option_context_new ("PLUGIN INFOS...");
  g_option_context_add_main_entries (context, entries, "Plugin Infos to XML");

  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_printerr ("Error: %s\n", error->message);
      exit (1);
    }

  if (opt_app == NULL)
    {
      g_printerr ("Error: application ID was not provided.\n");
      exit (1);
    }

  if (opt_data_dir == NULL)
    {
      g_printerr ("Error: install data dir was not provided.\n");
      exit (1);
    }

  if (opt_module_dir == NULL)
    {
      g_printerr ("Error: install module dir was not provided.\n");
      exit (1);
    }

  if (opt_output_dir == NULL)
    {
      g_printerr ("Error: output dir was not provided.\n");
      exit (1);
    }

  if (opt_files == NULL)
    {
      g_printerr ("Error: no files we given to convert.\n");
      exit (1);
    }

  g_type_init ();

  output_location = g_file_new_for_commandline_arg (opt_output_dir);
  plugins_location = g_file_get_child (output_location, "plugins");
  icons_location = g_file_get_child (output_location, "icons");

  to_app_location (&plugins_location);
  to_app_location (&icons_location);

  create_dir (output_location);
  create_dir (plugins_location);
  create_dir (icons_location);

  filename_tmp = g_strdup_printf ("%s-%s.xml", opt_app, PLUGIN_STORE_VERSION);
  filename = g_build_filename (opt_output_dir, filename_tmp, NULL);
  g_free (filename_tmp);

  writer = xmlNewTextWriterFilename (filename, FALSE);

  g_free (filename);

  if (writer == NULL)
    g_error ("Could not create xmlTextWriter");

  xmlTextWriterSetIndentString (writer, BAD_CAST "    ");

  if (xmlTextWriterStartDocument (writer, NULL, "UTF-8", NULL) < 0)
    g_error ("Could not write xml document start");

  xmlTextWriterStartElement (writer, BAD_CAST "application");
  xmlTextWriterWriteAttribute (writer, BAD_CAST "id", BAD_CAST opt_app);
  xmlTextWriterWriteAttribute (writer, BAD_CAST "version",
                               BAD_CAST PLUGIN_STORE_VERSION);

  xmlTextWriterSetIndent (writer, 1);
  xmlTextWriterWriteElement (writer, BAD_CAST "install-data-dir",
                             BAD_CAST opt_module_dir);
  xmlTextWriterWriteElement (writer, BAD_CAST "install-module-dir",
                             BAD_CAST opt_data_dir);

  xmlTextWriterStartElement (writer, BAD_CAST "plugins");
  xmlTextWriterSetIndent (writer, 2);

  for (i = 0; opt_files[i] != NULL; ++i)
    process_plugin (opt_files[i], writer);

  if (xmlTextWriterEndDocument (writer) < 0)
    g_error ("Failed to end xml document");

  xmlTextWriterFlush (writer);
  xmlFreeTextWriter (writer);

  g_object_unref (output_location);
  g_object_unref (plugins_location);
  g_object_unref (icons_location);
  g_option_context_free (context);

  return 0;
}
