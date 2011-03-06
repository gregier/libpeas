/*
 * peas-extension-python.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Steve Frécinaux
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

#include <girepository.h>
/* _POSIX_C_SOURCE is defined in Python.h and in limits.h included by
 * girepository.h, so we unset it here to avoid a warning. Yep, that's bad. */
#undef _POSIX_C_SOURCE
#include <Python.h>
#include <pygobject.h>
#include <libpeas/peas-introspection.h>
#include <libpeas/peas-extension-subclasses.h>
#include "peas-extension-python.h"

G_DEFINE_TYPE (PeasExtensionPython, peas_extension_python, PEAS_TYPE_EXTENSION);

enum {
  PROP_0,
  PROP_INSTANCE
};

static void
peas_extension_python_init (PeasExtensionPython *pyexten)
{
}

static void
peas_extension_python_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);
  PyGILState_STATE state;

  switch (prop_id)
    {
    case PROP_INSTANCE:
      pyexten->instance = g_value_get_pointer (value);

      state = pyg_gil_state_ensure ();
      Py_INCREF (pyexten->instance);
      pyg_gil_state_release (state);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
peas_extension_python_get_property (GObject      *object,
                                    guint         prop_id,
                                    GValue       *value,
                                    GParamSpec   *pspec)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);

  switch (prop_id)
    {
    case PROP_INSTANCE:
      g_value_set_pointer (value, pyexten->instance);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static gboolean
peas_extension_python_call (PeasExtension *exten,
                            GType          gtype,
                            const gchar   *method_name,
                            GIArgument    *args,
                            GIArgument    *retval)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (exten);
  PyGILState_STATE state;
  GObject *instance;
  gboolean success;

  if (gtype == G_TYPE_INVALID)
    gtype = peas_extension_get_extension_type (exten);

  state = pyg_gil_state_ensure ();

  instance = pygobject_get (pyexten->instance);
  success = peas_method_apply (instance, gtype, method_name, args, retval);

  pyg_gil_state_release (state);
  return success;
}

static void
peas_extension_python_dispose (GObject *object)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);

  if (pyexten->instance)
    {
      PyGILState_STATE state = pyg_gil_state_ensure ();

      Py_DECREF (pyexten->instance);
      pyexten->instance = NULL;

      pyg_gil_state_release (state);
    }

  G_OBJECT_CLASS (peas_extension_python_parent_class)->dispose (object);
}

static void
peas_extension_python_class_init (PeasExtensionPythonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasExtensionClass *extension_class = PEAS_EXTENSION_CLASS (klass);

  object_class->set_property = peas_extension_python_set_property;
  object_class->get_property = peas_extension_python_get_property;
  object_class->dispose = peas_extension_python_dispose;

  extension_class->call = peas_extension_python_call;

  g_object_class_install_property (object_class,
                                   PROP_INSTANCE,
                                   g_param_spec_pointer ("instance",
                                                        "Extension Instance",
                                                        "The Python Extension Instance",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

PeasExtension *
peas_extension_python_new (GType     gtype,
                           PyObject *instance)
{
  PeasExtensionPython *pyexten;
  GType real_type;

  real_type = peas_extension_register_subclass (PEAS_TYPE_EXTENSION_PYTHON, gtype);
  return PEAS_EXTENSION (g_object_new (real_type,
                                       "extension-type", gtype,
                                       "instance", instance,
                                       NULL));
}
