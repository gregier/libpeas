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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

G_DEFINE_TYPE (PeasExtensionPython, peas_extension_python, PEAS_TYPE_EXTENSION_WRAPPER);

static void
peas_extension_python_init (PeasExtensionPython *pyexten)
{
}

static gboolean
peas_extension_python_call (PeasExtensionWrapper *exten,
                            GType                 interface_type,
                            GICallableInfo       *method_info,
                            const gchar          *method_name,
                            GIArgument           *args,
                            GIArgument           *retval)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (exten);
  PyGILState_STATE state;
  GObject *instance;
  gboolean success;

  state = pyg_gil_state_ensure ();

  instance = pygobject_get (pyexten->instance);
  success = peas_gi_method_call (instance, method_info, interface_type,
                                 method_name, args, retval);

  pyg_gil_state_release (state);
  return success;
}

static void
peas_extension_python_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);
  PyGILState_STATE state;
  GObject *instance;

  /* Don't add properties as they could shadow the instance's */

  state = pyg_gil_state_ensure ();

  instance = pygobject_get (pyexten->instance);
  g_object_set_property (instance, pspec->name, value);

  pyg_gil_state_release (state);
}

static void
peas_extension_python_get_property (GObject      *object,
                                    guint         prop_id,
                                    GValue       *value,
                                    GParamSpec   *pspec)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);
  PyGILState_STATE state;
  GObject *instance;

  /* Don't add properties as they could shadow the instance's */

  state = pyg_gil_state_ensure ();

  instance = pygobject_get (pyexten->instance);
  g_object_get_property (instance, pspec->name, value);

  pyg_gil_state_release (state);
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
  PeasExtensionWrapperClass *extension_class = PEAS_EXTENSION_WRAPPER_CLASS (klass);

  object_class->dispose = peas_extension_python_dispose;
  object_class->get_property = peas_extension_python_get_property;
  object_class->set_property = peas_extension_python_set_property;

  extension_class->call = peas_extension_python_call;
}

GObject *
peas_extension_python_new (GType     exten_type,
                           GType    *interfaces,
                           PyObject *instance)
{
  PeasExtensionPython *pyexten;
  GType real_type;

  real_type = peas_extension_register_subclass (PEAS_TYPE_EXTENSION_PYTHON,
                                                interfaces);

  /* Already Warned */
  if (real_type == G_TYPE_INVALID)
    {
      g_free (interfaces);
      return NULL;
    }

  pyexten = PEAS_EXTENSION_PYTHON (g_object_new (real_type, NULL));

  pyexten->instance = instance;
  PEAS_EXTENSION_WRAPPER (pyexten)->exten_type = exten_type;
  PEAS_EXTENSION_WRAPPER (pyexten)->interfaces = interfaces;
  Py_INCREF (instance);

  return G_OBJECT (pyexten);
}
