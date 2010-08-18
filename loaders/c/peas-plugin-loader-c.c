/*
 * peas-plugin-loader-c.c
 * This file is part of libpeas
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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
#include <gmodule.h>

#include "peas-plugin-loader-c.h"
#include "peas-extension-c.h"
#include <libpeas/peas-object-module.h>
#include <libpeas/peas-extension-base.h>

struct _PeasPluginLoaderCPrivate
{
  GHashTable *loaded_plugins;
};

G_DEFINE_TYPE (PeasPluginLoaderC, peas_plugin_loader_c, PEAS_TYPE_PLUGIN_LOADER);

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_PLUGIN_LOADER,
                                              PEAS_TYPE_PLUGIN_LOADER_C);
}

static void
peas_plugin_loader_c_add_module_directory (PeasPluginLoader *loader,
                                           const gchar      *module_dir)
{
  /* This is a no-op for C modules... */
}

static gboolean
peas_plugin_loader_c_load (PeasPluginLoader *loader,
                           PeasPluginInfo   *info)
{
  PeasPluginLoaderC *cloader = PEAS_PLUGIN_LOADER_C (loader);
  PeasObjectModule *module;
  const gchar *module_name;

  module = (PeasObjectModule *) g_hash_table_lookup (cloader->priv->loaded_plugins,
                                                     info);
  module_name = peas_plugin_info_get_module_name (info);

  if (module == NULL)
    {
      /* For now we force all modules to be resident */
      module = peas_object_module_new (module_name,
                                       peas_plugin_info_get_module_dir (info),
                                       TRUE);

      /* Infos are available for all the lifetime of the loader.
       * If this changes, we should use weak refs or something */

      g_hash_table_insert (cloader->priv->loaded_plugins, info, module);
      g_debug ("Insert module '%s' into C module set", module_name);
    }

  if (!g_type_module_use (G_TYPE_MODULE (module)))
    {
      g_warning ("Could not load plugin module: '%s'",
                 peas_plugin_info_get_name (info));

      return FALSE;
    }

  return TRUE;
}

static gboolean
peas_plugin_loader_c_provides_extension  (PeasPluginLoader *loader,
                                          PeasPluginInfo   *info,
                                          GType             exten_type)
{
  PeasPluginLoaderC *cloader = PEAS_PLUGIN_LOADER_C (loader);
  PeasObjectModule *module;

  module = (PeasObjectModule *) g_hash_table_lookup (cloader->priv->loaded_plugins,
                                                     info);
  g_return_val_if_fail (module != NULL, FALSE);

  return peas_object_module_provides_object (module, exten_type);
}

static PeasExtension *
peas_plugin_loader_c_create_extension (PeasPluginLoader *loader,
                                       PeasPluginInfo   *info,
                                       GType             exten_type,
                                       guint             n_parameters,
                                       GParameter       *parameters)
{
  PeasPluginLoaderC *cloader = PEAS_PLUGIN_LOADER_C (loader);
  PeasObjectModule *module;
  GParameter *exten_parameters;
  gpointer instance;

  module = (PeasObjectModule *) g_hash_table_lookup (cloader->priv->loaded_plugins,
                                                     info);
  g_return_val_if_fail (module != NULL, NULL);

  /* We want to add a "plugin-info" property so we can pass it to the extension
   * if it inherits from PeasExtensionBase. No need to actually "duplicate" the
   * GValues, a memcpy is sufficient as the source GValues are longer lived
   * than our local copy. */
  exten_parameters = g_new (GParameter, n_parameters + 1);
  memcpy (exten_parameters, parameters, sizeof (GParameter) * n_parameters);

  /* Initialize our additional property */
  exten_parameters[n_parameters].name = g_intern_static_string ("plugin-info");
  memset (&exten_parameters[n_parameters].value, 0, sizeof (GValue));
  g_value_init (&exten_parameters[n_parameters].value, PEAS_TYPE_PLUGIN_INFO);
  g_value_set_boxed (&exten_parameters[n_parameters].value, info);

  instance = peas_object_module_create_object (module,
                                               exten_type,
                                               n_parameters + 1,
                                               exten_parameters);

  g_value_unset (&exten_parameters[n_parameters].value);
  g_free (exten_parameters);

  if (instance == NULL)
    {
      g_debug ("Plugin '%s' does not provide a '%s' extension",
               peas_plugin_info_get_module_name (info),
               g_type_name (exten_type));
      return NULL;
    }

  g_return_val_if_fail (G_IS_OBJECT (instance), NULL);
  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (instance, exten_type), NULL);

  return peas_extension_c_new (exten_type, G_OBJECT (instance));
}

static void
peas_plugin_loader_c_unload (PeasPluginLoader *loader,
                             PeasPluginInfo   *info)
{
  PeasPluginLoaderC *cloader = PEAS_PLUGIN_LOADER_C (loader);
  PeasObjectModule *module;

  module = (PeasObjectModule *) g_hash_table_lookup (cloader->priv->loaded_plugins,
                                                     info);

  g_debug ("Unloading plugin '%s'", peas_plugin_info_get_module_name (info));
  g_type_module_unuse (G_TYPE_MODULE (module));
}

static void
peas_plugin_loader_c_init (PeasPluginLoaderC *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            PEAS_TYPE_PLUGIN_LOADER_C,
                                            PeasPluginLoaderCPrivate);

  /* loaded_plugins maps PeasPluginInfo to a PeasObjectModule */
  self->priv->loaded_plugins = g_hash_table_new (g_direct_hash,
                                                 g_direct_equal);
}

static void
peas_plugin_loader_c_finalize (GObject *object)
{
  PeasPluginLoaderC *cloader = PEAS_PLUGIN_LOADER_C (object);
  GList *infos;
  GList *item;

  /* FIXME: this sanity check it's not efficient. Let's remove it
   * once we are confident with the code */

  infos = g_hash_table_get_keys (cloader->priv->loaded_plugins);

  for (item = infos; item; item = item->next)
    {
      PeasPluginInfo *info = (PeasPluginInfo *) item->data;

      if (peas_plugin_info_is_loaded (info))
        {
          g_warning ("There are still C plugins loaded during destruction");
          break;
        }
    }

  g_list_free (infos);

  g_hash_table_destroy (cloader->priv->loaded_plugins);

  G_OBJECT_CLASS (peas_plugin_loader_c_parent_class)->finalize (object);
}

static void
peas_plugin_loader_c_class_init (PeasPluginLoaderCClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasPluginLoaderClass *loader_class = PEAS_PLUGIN_LOADER_CLASS (klass);

  object_class->finalize = peas_plugin_loader_c_finalize;

  loader_class->add_module_directory = peas_plugin_loader_c_add_module_directory;
  loader_class->load = peas_plugin_loader_c_load;
  loader_class->unload = peas_plugin_loader_c_unload;
  loader_class->provides_extension = peas_plugin_loader_c_provides_extension;
  loader_class->create_extension = peas_plugin_loader_c_create_extension;

  g_type_class_add_private (object_class, sizeof (PeasPluginLoaderCPrivate));
}
