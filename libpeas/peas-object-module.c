/*
 * peas-object-module.c
 * This file is part of libpeas
 *
 * Copyright (C) 2003 Marco Pesenti Gritti
 * Copyright (C) 2003-2004 Christian Persch
 * Copyright (C) 2005-2007 Paolo Maggi
 * Copyright (C) 2008 Jesse van den Kieboom
 * Copyright (C) 2010 Steve Frécinaux
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

#include "peas-object-module.h"
#include "peas-plugin-loader.h"

/**
 * SECTION:peas-object-module
 * @short_description: Type module which allows extension registration.
 *
 * #PeasObjectModule is a subclass of #GTypeModule which allows registration
 * of extensions.  It will be used by C extensions implementors to register
 * extension implementations from within the peas_register_types module
 * function.
 **/

typedef void (*PeasObjectModuleRegisterFunc) (PeasObjectModule *module);

enum {
  PROP_0,
  PROP_MODULE_NAME,
  PROP_PATH,
  PROP_SYMBOL,
  PROP_RESIDENT,
  PROP_LOCAL_LINKAGE,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

typedef struct {
  GType iface_type;
  PeasFactoryFunc func;
  gpointer user_data;
  GDestroyNotify destroy_func;
} InterfaceImplementation;

struct _PeasObjectModulePrivate {
  GModule *library;

  PeasObjectModuleRegisterFunc register_func;
  GArray *implementations;

  gchar *path;
  gchar *module_name;
  gchar *symbol;

  guint resident : 1;
  guint local_linkage : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (PeasObjectModule,
                            peas_object_module,
                            G_TYPE_TYPE_MODULE)

#define GET_PRIV(o) \
  (peas_object_module_get_instance_private (o))

#define TYPE_MISSING_PLUGIN_INFO_PROPERTY (G_TYPE_FLAG_RESERVED_ID_BIT)

static const gchar *intern_plugin_info = NULL;

static gboolean
peas_object_module_load (GTypeModule *gmodule)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (gmodule);
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  g_return_val_if_fail (priv->module_name != NULL, FALSE);

  if (priv->path == NULL)
    {
      g_return_val_if_fail (priv->resident, FALSE);
      g_return_val_if_fail (!priv->local_linkage, FALSE);

      priv->library = g_module_open (NULL, 0);
    }
  else
    {
      gchar *path;
      GModuleFlags flags = 0;

      path = g_module_build_path (priv->path, priv->module_name);

      /* g_module_build_path() will add G_MODULE_SUFFIX to the path,
       * however g_module_open() will only try to load the libtool archive
       * if there is no suffix specified. So we remove G_MODULE_SUFFIX here
       * which allows uninstalled builds to load plugins as well, as there
       * is only the .la file in the build directory.
       */
      if (G_MODULE_SUFFIX[0] != '\0' && g_str_has_suffix (path, "." G_MODULE_SUFFIX))
        path[strlen (path) - strlen (G_MODULE_SUFFIX) - 1] = '\0';

      if (priv->local_linkage)
        flags = G_MODULE_BIND_LOCAL;

      /* Bind symbols immediately to avoid errors long after loading */
      priv->library = g_module_open (path, flags);
      g_free (path);
    }

  if (priv->library == NULL)
    {
      g_warning ("Failed to load module '%s': %s",
                 priv->module_name, g_module_error ());

      return FALSE;
    }

  /* Extract the required symbol from the library */
  if (!g_module_symbol (priv->library, priv->symbol,
                        (gpointer) &priv->register_func))
    {
      g_warning ("Failed to get '%s' for module '%s': %s",
                 priv->symbol, priv->module_name, g_module_error ());
      g_module_close (priv->library);

      return FALSE;
    }

  /* The symbol can still be NULL even
   * though g_module_symbol() returned TRUE
   */
  if (priv->register_func == NULL)
    {
      g_warning ("Invalid '%s' in module '%s'",
                 priv->symbol, priv->module_name);
      g_module_close (priv->library);

      return FALSE;
    }

  if (priv->resident)
    g_module_make_resident (priv->library);

  priv->register_func (module);

  return TRUE;
}

static void
peas_object_module_unload (GTypeModule *gmodule)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (gmodule);
  PeasObjectModulePrivate *priv = GET_PRIV (module);
  InterfaceImplementation *impls;
  guint i;

  g_module_close (priv->library);

  priv->library = NULL;
  priv->register_func = NULL;

  impls = (InterfaceImplementation *) priv->implementations->data;
  for (i = 0; i < priv->implementations->len; ++i)
    {
      if (impls[i].destroy_func != NULL)
        impls[i].destroy_func (impls[i].user_data);
    }

  g_array_remove_range (priv->implementations, 0,
                        priv->implementations->len);
}

static void
peas_object_module_init (PeasObjectModule *module)
{
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  priv->implementations = g_array_new (FALSE, FALSE,
                                       sizeof (InterfaceImplementation));
}

static void
peas_object_module_finalize (GObject *object)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (object);
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  g_free (priv->path);
  g_free (priv->module_name);
  g_free (priv->symbol);
  g_array_unref (priv->implementations);

  G_OBJECT_CLASS (peas_object_module_parent_class)->finalize (object);
}

static void
peas_object_module_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (object);
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      g_value_set_string (value, priv->module_name);
      break;
    case PROP_PATH:
      g_value_set_string (value, priv->path);
      break;
    case PROP_SYMBOL:
      g_value_set_string (value, priv->symbol);
      break;
    case PROP_RESIDENT:
      g_value_set_boolean (value, priv->resident);
      break;
    case PROP_LOCAL_LINKAGE:
      g_value_set_boolean (value, priv->local_linkage);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_object_module_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (object);
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      priv->module_name = g_value_dup_string (value);
      g_type_module_set_name (G_TYPE_MODULE (object),
                              priv->module_name);
      break;
    case PROP_PATH:
      priv->path = g_value_dup_string (value);
      break;
    case PROP_SYMBOL:
      priv->symbol = g_value_dup_string (value);
      break;
    case PROP_RESIDENT:
      priv->resident = g_value_get_boolean (value);
      break;
    case PROP_LOCAL_LINKAGE:
      priv->local_linkage = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_object_module_class_init (PeasObjectModuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (klass);

  intern_plugin_info = g_intern_static_string ("plugin-info");

  object_class->set_property = peas_object_module_set_property;
  object_class->get_property = peas_object_module_get_property;
  object_class->finalize = peas_object_module_finalize;

  module_class->load = peas_object_module_load;
  module_class->unload = peas_object_module_unload;

  properties[PROP_MODULE_NAME] =
    g_param_spec_string ("module-name",
                         "Module Name",
                         "The module to load for this object",
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  properties[PROP_PATH] =
    g_param_spec_string ("path",
                         "Path",
                         "The path to use when loading this module",
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  properties[PROP_SYMBOL] =
    g_param_spec_string ("symbol",
                         "Symbol",
                         "The registration symbol to use for this module",
                         "peas_register_types",
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  properties[PROP_RESIDENT] =
    g_param_spec_boolean ("resident",
                          "Resident",
                          "Whether the module is resident",
                          FALSE,
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

  /**
   * PeasObjectModule:local-linkage
   *
   * This property indicates whether the module is loaded with
   * local linkage, i.e. #G_MODULE_BIND_LOCAL.
   *
   * Since 1.14
   */
  properties[PROP_LOCAL_LINKAGE] =
    g_param_spec_boolean ("local-linkage",
                          "Local linkage",
                          "Whether the module loaded with local linkage",
                          FALSE,
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

/**
 * peas_object_module_new: (skip)
 * @module_name: The module name.
 * @path: The path.
 * @resident: If the module should be resident.
 *
 * Creates a new #PeasObjectModule.
 *
 * Return value: a new #PeasObjectModule.
 */
PeasObjectModule *
peas_object_module_new (const gchar *module_name,
                        const gchar *path,
                        gboolean     resident)
{
  g_return_val_if_fail (module_name != NULL && *module_name != '\0', NULL);
  g_return_val_if_fail (path != NULL && *path != '\0', NULL);

  return PEAS_OBJECT_MODULE (g_object_new (PEAS_TYPE_OBJECT_MODULE,
                                           "module-name", module_name,
                                           "path", path,
                                           "resident", resident,
                                           NULL));
}

/**
 * peas_object_module_new_full: (skip)
 * @module_name: The module name.
 * @path: The path.
 * @resident: If the module should be resident.
 * @local_linkage: Whether to load the module with local linkage.
 *
 * Creates a new #PeasObjectModule.
 *
 * Return value: a new #PeasObjectModule.
 *
 * Since 1.14
 */
PeasObjectModule *
peas_object_module_new_full (const gchar *module_name,
                             const gchar *path,
                             gboolean     resident,
                             gboolean     local_linkage)
{
  g_return_val_if_fail (module_name != NULL && *module_name != '\0', NULL);
  g_return_val_if_fail (path != NULL && *path != '\0', NULL);

  return PEAS_OBJECT_MODULE (g_object_new (PEAS_TYPE_OBJECT_MODULE,
                                           "module-name", module_name,
                                           "path", path,
                                           "resident", resident,
                                           "local-linkage", local_linkage,
                                           NULL));
}

/**
 * peas_object_module_new_embedded: (skip)
 * @module_name: The module name.
 *
 * Creates a new #PeasObjectModule for an embedded plugin.
 *
 * Return value: a new #PeasObjectModule.
 *
 * Since: 1.18
 */
PeasObjectModule *
peas_object_module_new_embedded (const gchar *module_name,
                                 const gchar *symbol)
{
  g_return_val_if_fail (module_name != NULL && *module_name != '\0', NULL);
  g_return_val_if_fail (symbol != NULL && *symbol != '\0', NULL);

  return PEAS_OBJECT_MODULE (g_object_new (PEAS_TYPE_OBJECT_MODULE,
                                           "module-name", module_name,
                                           "symbol", symbol,
                                           "resident", TRUE,
                                           "local-linkage", FALSE,
                                           NULL));
}

/**
 * peas_object_module_create_object: (skip)
 * @module: A #PeasObjectModule.
 * @interface: The #GType of the extension interface.
 * @n_parameters: The number of paramteters.
 * @parameters: (array length=n_parameters): The parameters.
 *
 * Creates an object for the @interface passing @n_parameters
 * and @parameters to the #PeasFactoryFunc. If @module does
 * not provide a #PeasFactoryFunc for @interface then
 * %NULL is returned.
 *
 * Return value: (transfer full): The created object, or %NULL.
 */
GObject *
peas_object_module_create_object (PeasObjectModule *module,
                                  GType             interface,
                                  guint             n_parameters,
                                  GParameter       *parameters)
{
  PeasObjectModulePrivate *priv = GET_PRIV (module);
  guint i;
  InterfaceImplementation *impls;

  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), NULL);

  if (interface != PEAS_TYPE_PLUGIN_LOADER)
    g_return_val_if_fail (G_TYPE_IS_INTERFACE (interface), NULL);

  impls = (InterfaceImplementation *) priv->implementations->data;
  for (i = 0; i < priv->implementations->len; ++i)
    {
      if (impls[i].iface_type == interface)
        return impls[i].func (n_parameters, parameters, impls[i].user_data);
    }

  return NULL;
}

/**
 * peas_object_module_provides_object: (skip)
 * @module: A #PeasObjectModule.
 * @interface: The #GType of the extension interface.
 *
 * Determines if the module provides an extension for @interface.
 *
 * Return value: if the module provides an extension for @interface.
 */
gboolean
peas_object_module_provides_object (PeasObjectModule *module,
                                    GType             interface)
{
  PeasObjectModulePrivate *priv = GET_PRIV (module);
  guint i;
  InterfaceImplementation *impls;

  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), FALSE);

  if (interface != PEAS_TYPE_PLUGIN_LOADER)
    g_return_val_if_fail (G_TYPE_IS_INTERFACE (interface), FALSE);

  impls = (InterfaceImplementation *) priv->implementations->data;
  for (i = 0; i < priv->implementations->len; ++i)
    {
      if (impls[i].iface_type == interface)
        return TRUE;
    }

  return FALSE;
}

/**
 * peas_object_module_get_path: (skip)
 * @module: A #PeasObjectModule.
 *
 * Gets the path.
 *
 * Return value: the path.
 */
const gchar *
peas_object_module_get_path (PeasObjectModule *module)
{
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), NULL);

  return priv->path;
}

/**
 * peas_object_module_get_module_name: (skip)
 * @module: A #PeasObjectModule.
 *
 * Gets the module name.
 *
 * Return value: the module name.
 */
const gchar *
peas_object_module_get_module_name (PeasObjectModule *module)
{
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), NULL);

  return priv->module_name;
}

/**
 * peas_object_module_get_symbol: (skip)
 * @module: A #PeasObjectModule.
 *
 * Gets the symbol name used to register extension implementations.
 *
 * Return value: the symbol name.
 *
 * Since: 1.18
 */
const gchar *
peas_object_module_get_symbol (PeasObjectModule *module)
{
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), NULL);

  return priv->symbol;
}

/**
 * peas_object_module_get_library: (skip)
 * @module: A #PeasObjectModule.
 *
 * Gets the library.
 *
 * Return value: the library.
 */
GModule *
peas_object_module_get_library (PeasObjectModule *module)
{
  PeasObjectModulePrivate *priv = GET_PRIV (module);

  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), NULL);

  return priv->library;
}

/**
 * peas_object_module_register_extension_factory:
 * @module: Your plugin's #PeasObjectModule.
 * @iface_type: The #GType of the extension interface you implement.
 * @factory_func: The #PeasFactoryFunc that will create the @iface_type
 *   instance when requested.
 * @user_data: Data to pass to @func calls.
 * @destroy_func: A #GDestroyNotify for @user_data.
 *
 * Register an implementation for an extension type through a factory
 * function @factory_func which will instantiate the extension when
 * requested.
 *
 * This method is primarily meant to be used by native bindings (like gtkmm),
 * creating native types which cannot be instantiated correctly using
 * g_object_new().  For other uses, you will usually prefer relying on
 * peas_object_module_register_extension_type().
 */
void
peas_object_module_register_extension_factory (PeasObjectModule *module,
                                               GType             iface_type,
                                               PeasFactoryFunc   factory_func,
                                               gpointer          user_data,
                                               GDestroyNotify    destroy_func)
{
  PeasObjectModulePrivate *priv = GET_PRIV (module);
  InterfaceImplementation impl = { iface_type, factory_func, user_data, destroy_func };

  g_return_if_fail (PEAS_IS_OBJECT_MODULE (module));
  g_return_if_fail (!peas_object_module_provides_object (module, iface_type));
  g_return_if_fail (factory_func != NULL);

  if (iface_type != PEAS_TYPE_PLUGIN_LOADER)
    g_return_if_fail (G_TYPE_IS_INTERFACE (iface_type));

  g_array_append_val (priv->implementations, impl);

  g_debug ("Registered extension for type '%s'", g_type_name (iface_type));
}

static GObject *
create_gobject_from_type (guint       n_parameters,
                          GParameter *parameters,
                          gpointer    user_data)
{
  GType exten_type = GPOINTER_TO_SIZE (user_data);

  /* We should be called with a "plugin-info" property appended
   * to the parameters. Let's get rid of it if the actual type
   * doesn't have such a property as it would cause a warning.
   */
  if ((exten_type & TYPE_MISSING_PLUGIN_INFO_PROPERTY) != 0)
    {
      exten_type &= ~TYPE_MISSING_PLUGIN_INFO_PROPERTY;

      if (n_parameters > 0)
        {
          GParameter *info_param = &parameters[n_parameters - 1];

          if (info_param->name == intern_plugin_info &&
              G_VALUE_TYPE (&info_param->value) == PEAS_TYPE_PLUGIN_INFO)
            {
              n_parameters--;
            }
        }
    }

  return G_OBJECT (g_object_newv (exten_type, n_parameters, parameters));
}

/**
 * peas_object_module_register_extension_type:
 * @module: Your plugin's #PeasObjectModule.
 * @iface_type: The #GType of the extension interface you implement.
 * @extension_type: The #GType of your implementation of @iface_type.
 *
 * Register an extension type which implements the extension interface
 * @iface_type.
 */
void
peas_object_module_register_extension_type (PeasObjectModule *module,
                                            GType             iface_type,
                                            GType             extension_type)
{
  GObjectClass *cls;
  GParamSpec *pspec;

  g_return_if_fail (PEAS_IS_OBJECT_MODULE (module));
  g_return_if_fail (!peas_object_module_provides_object (module, iface_type));
  g_return_if_fail (g_type_is_a (extension_type, iface_type));

  if (iface_type != PEAS_TYPE_PLUGIN_LOADER)
    g_return_if_fail (G_TYPE_IS_INTERFACE (iface_type));

  cls = g_type_class_ref (extension_type);
  pspec = g_object_class_find_property (cls, "plugin-info");

  /* Avoid checking for this each time in the factory function */
  if (pspec == NULL || pspec->value_type != PEAS_TYPE_PLUGIN_INFO)
    extension_type |= TYPE_MISSING_PLUGIN_INFO_PROPERTY;

  peas_object_module_register_extension_factory (module,
                                                 iface_type,
                                                 create_gobject_from_type,
                                                 GSIZE_TO_POINTER (extension_type),
                                                 NULL);

  g_type_class_unref (cls);
}
