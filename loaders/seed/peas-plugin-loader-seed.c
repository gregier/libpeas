/*
 * peas-plugin-loader-seed.c
 * This file is part of libpeas
 *
 * Copyright (C) 2009 - Steve Frécinaux
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

#include <seed.h>

#include "peas-plugin-loader-seed.h"
#include "peas-seed-plugin.h"
#include "config.h"

G_DEFINE_DYNAMIC_TYPE (PeasPluginLoaderSeed, peas_plugin_loader_seed, PEAS_TYPE_PLUGIN_LOADER);

static SeedEngine *seed = NULL;

static void
peas_plugin_loader_seed_add_module_directory (PeasPluginLoader *loader,
                                              const gchar      *module_dir)
{
  gchar **sp = seed_engine_get_search_path (seed);

  if (sp)
    {
      gchar *orig_sp = g_strjoinv (":", sp);
      gchar *new_sp = g_strconcat (module_dir, ":", orig_sp, NULL);

      seed_engine_set_search_path (seed, new_sp);

      g_free (new_sp);
      g_free (orig_sp);
    }
  else
    {
      seed_engine_set_search_path (seed, module_dir);
    }
}

static gchar *
get_script_for_plugin_info (PeasPluginInfo   *info,
                            SeedContext       context)
{
  gchar *basename;
  gchar *filename;
  gchar *script = NULL;

  basename = g_strconcat (peas_plugin_info_get_module_name (info), ".js", NULL);
  filename = g_build_filename (peas_plugin_info_get_module_dir (info), basename, NULL);

  g_debug ("Seed script filename is %s", filename);

  g_file_get_contents (filename, &script, NULL, NULL);

  g_free (basename);
  g_free (filename);

  return script;
}

static PeasPlugin *
peas_plugin_loader_seed_load (PeasPluginLoader *loader,
                              PeasPluginInfo   *info)
{
  SeedContext context;
  gchar *script;
  SeedObject global;
  SeedValue plugin_object;
  SeedException exc = NULL;
  PeasPlugin *plugin = NULL;

  context = seed_context_create (seed->group, NULL);

  seed_prepare_global_context (context);
  script = get_script_for_plugin_info (info, context);

  seed_simple_evaluate (context, script, &exc);
  g_free (script);

  if (exc)
    {
      gchar *exc_string = seed_exception_to_string (context, exc);
      g_warning ("Seed Exception: %s", exc_string);
      g_free (exc_string);
    }
  else
    {
      global = seed_context_get_global_object (context);
      plugin_object = seed_object_get_property (context, global, "plugin");

      if (seed_value_is_object (context, plugin_object))
        plugin = peas_seed_plugin_new (info, context, plugin_object);
    }

  seed_context_unref (context);

  return plugin;
}

static void
peas_plugin_loader_seed_unload (PeasPluginLoader *loader,
                                PeasPluginInfo   *info)
{
}

static void
peas_plugin_loader_seed_garbage_collect (PeasPluginLoader *loader)
{
  seed_context_collect (seed->context);
}

static void
peas_plugin_loader_seed_init (PeasPluginLoaderSeed *sloader)
{
  /* This is somewhat buggy as the seed engine cannot be reinitialized
   * and is shared among instances (esp wrt module paths), but apparently there
   * is no way to avoid having it shared... */
  if (!seed)
    seed = seed_init (NULL, NULL);
}

static void
peas_plugin_loader_seed_class_init (PeasPluginLoaderSeedClass *klass)
{
  PeasPluginLoaderClass *loader_class = PEAS_PLUGIN_LOADER_CLASS (klass);

  loader_class->add_module_directory = peas_plugin_loader_seed_add_module_directory;
  loader_class->load = peas_plugin_loader_seed_load;
  loader_class->unload = peas_plugin_loader_seed_unload;
  loader_class->garbage_collect = peas_plugin_loader_seed_garbage_collect;
}

static void
peas_plugin_loader_seed_class_finalize (PeasPluginLoaderSeedClass *klass)
{
}

G_MODULE_EXPORT GObject *
register_peas_plugin_loader (GTypeModule *type_module)
{
  peas_plugin_loader_seed_register_type (type_module);
  peas_seed_plugin_register_type_ext (type_module);

  return g_object_new (PEAS_TYPE_PLUGIN_LOADER_SEED, NULL);
}
