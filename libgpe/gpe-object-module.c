/*
 * gpe-object-module.c
 * This file is part of libgpe
 *
 * Copyright (C) 2003 Marco Pesenti Gritti
 * Copyright (C) 2003-2004 Christian Persch
 * Copyright (C) 2005-2007 Paolo Maggi
 * Copyright (C) 2008 Jesse van den Kieboom
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

#include "config.h"

#include "gpe-object-module.h"

typedef GType (*GPEObjectModuleRegisterFunc) (GTypeModule *);

enum {
	PROP_0,
	PROP_MODULE_NAME,
	PROP_PATH,
	PROP_TYPE_REGISTRATION,
	PROP_RESIDENT
};

struct _GPEObjectModulePrivate
{
	GModule *library;

	GType type;
	gchar *path;
	gchar *module_name;
	gchar *type_registration;

	gboolean resident;
};

G_DEFINE_TYPE (GPEObjectModule, gpe_object_module, G_TYPE_TYPE_MODULE);

static gboolean
gpe_object_module_load (GTypeModule *gmodule)
{
	GPEObjectModule *module = GPE_OBJECT_MODULE (gmodule);
	GPEObjectModuleRegisterFunc register_func;
	gchar *path;

	path = g_module_build_path (module->priv->path, module->priv->module_name);
	g_return_val_if_fail (path != NULL, FALSE);

	module->priv->library = g_module_open (path,
					       G_MODULE_BIND_LAZY);
	g_free (path);

	if (module->priv->library == NULL)
	{
		g_warning ("%s: %s", module->priv->module_name, g_module_error());

		return FALSE;
	}

	/* extract symbols from the lib */
	if (!g_module_symbol (module->priv->library, module->priv->type_registration,
			      (void *) &register_func))
	{
		g_warning ("%s: %s", module->priv->module_name, g_module_error());
		g_module_close (module->priv->library);

		return FALSE;
	}

	/* symbol can still be NULL even though g_module_symbol
	 * returned TRUE */
	if (register_func == NULL)
	{
		g_warning ("Symbol '%s' should not be NULL", module->priv->type_registration);
		g_module_close (module->priv->library);

		return FALSE;
	}

	module->priv->type = register_func (gmodule);

	if (module->priv->type == 0)
	{
		g_warning ("Invalid object contained by module %s", module->priv->module_name);
		return FALSE;
	}

	if (module->priv->resident)
	{
		g_module_make_resident (module->priv->library);
	}

	return TRUE;
}

static void
gpe_object_module_unload (GTypeModule *gmodule)
{
	GPEObjectModule *module = GPE_OBJECT_MODULE (gmodule);

	g_module_close (module->priv->library);

	module->priv->library = NULL;
	module->priv->type = 0;
}

static void
gpe_object_module_init (GPEObjectModule *module)
{
	module->priv = G_TYPE_INSTANCE_GET_PRIVATE (module,
						    GPE_TYPE_OBJECT_MODULE,
						    GPEObjectModulePrivate);
}

static void
gpe_object_module_finalize (GObject *object)
{
	GPEObjectModule *module = GPE_OBJECT_MODULE (object);

	g_free (module->priv->path);
	g_free (module->priv->module_name);
	g_free (module->priv->type_registration);

	G_OBJECT_CLASS (gpe_object_module_parent_class)->finalize (object);
}

static void
gpe_object_module_get_property (GObject    *object,
			        guint       prop_id,
			        GValue     *value,
			        GParamSpec *pspec)
{
	GPEObjectModule *module = GPE_OBJECT_MODULE (object);

	switch (prop_id)
	{
		case PROP_MODULE_NAME:
			g_value_set_string (value, module->priv->module_name);
			break;
		case PROP_PATH:
			g_value_set_string (value, module->priv->path);
			break;
		case PROP_TYPE_REGISTRATION:
			g_value_set_string (value, module->priv->type_registration);
			break;
		case PROP_RESIDENT:
			g_value_set_boolean (value, module->priv->resident);
			break;
		default:
			g_return_if_reached ();
	}
}

static void
gpe_object_module_set_property (GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	GPEObjectModule *module = GPE_OBJECT_MODULE (object);

	switch (prop_id)
	{
		case PROP_MODULE_NAME:
			module->priv->module_name = g_value_dup_string (value);
			g_type_module_set_name (G_TYPE_MODULE (object),
						module->priv->module_name);
			break;
		case PROP_PATH:
			module->priv->path = g_value_dup_string (value);
			break;
		case PROP_TYPE_REGISTRATION:
			module->priv->type_registration = g_value_dup_string (value);
			break;
		case PROP_RESIDENT:
			module->priv->resident = g_value_get_boolean (value);
			break;
		default:
			g_return_if_reached ();
	}
}

static void
gpe_object_module_class_init (GPEObjectModuleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (klass);

	object_class->set_property = gpe_object_module_set_property;
	object_class->get_property = gpe_object_module_get_property;
	object_class->finalize = gpe_object_module_finalize;

	module_class->load = gpe_object_module_load;
	module_class->unload = gpe_object_module_unload;

	g_object_class_install_property (object_class,
					 PROP_MODULE_NAME,
					 g_param_spec_string ("module-name",
							      "Module Name",
							      "The module to load for this object",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_PATH,
					 g_param_spec_string ("path",
							      "Path",
							      "The path to use when loading this module",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_TYPE_REGISTRATION,
					 g_param_spec_string ("type-registration",
							      "Type Registration",
							      "The name of the type registration function",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_RESIDENT,
					 g_param_spec_boolean ("resident",
							       "Resident",
							       "Whether the module is resident",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (klass, sizeof (GPEObjectModulePrivate));
}

GPEObjectModule *
gpe_object_module_new (const gchar *module_name,
		       const gchar *path,
		       const gchar *type_registration,
		       gboolean     resident)
{
	return (GPEObjectModule *)g_object_new (GPE_TYPE_OBJECT_MODULE,
						  "module-name",
						  module_name,
						  "path",
						  path,
						  "type-registration",
						  type_registration,
						  "resident",
						  resident,
						  NULL);
}

GObject *
gpe_object_module_new_object (GPEObjectModule *module,
			      const gchar            *first_property_name,
			      ...)
{
	va_list var_args;
	GObject *result;

	g_return_val_if_fail (module->priv->type != 0, NULL);

	va_start (var_args, first_property_name);
	result = g_object_new_valist (module->priv->type, first_property_name, var_args);
	va_end (var_args);

	return result;
}

const gchar *
gpe_object_module_get_path (GPEObjectModule *module)
{
	g_return_val_if_fail (GPE_IS_OBJECT_MODULE (module), NULL);

	return module->priv->path;
}

const gchar *
gpe_object_module_get_module_name (GPEObjectModule *module)
{
	g_return_val_if_fail (GPE_IS_OBJECT_MODULE (module), NULL);

	return module->priv->module_name;
}

const gchar *
gpe_object_module_get_type_registration (GPEObjectModule *module)
{
	g_return_val_if_fail (GPE_IS_OBJECT_MODULE (module), NULL);

	return module->priv->type_registration;
}

GType
gpe_object_module_get_object_type (GPEObjectModule *module)
{
	g_return_val_if_fail (GPE_IS_OBJECT_MODULE (module), 0);

	return module->priv->type;
}
