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

static void
peas_extension_python_init (PeasExtensionPython *pyexten)
{
}

static gboolean
peas_extension_python_call (PeasExtension *exten,
                            GType          gtype,
                            const gchar   *method_name,
                            GIArgument    *args,
                            GIArgument    *retval)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (exten);
  GObject *instance;

  if (gtype == G_TYPE_INVALID)
    gtype = peas_extension_get_extension_type (exten);

  instance = pygobject_get (pyexten->instance);

  return peas_method_apply (instance, gtype, method_name, args, retval);
}

static void
peas_extension_python_finalize (GObject *object)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);

  if (pyexten->instance)
    {
      Py_DECREF (pyexten->instance);
    }

  G_OBJECT_CLASS (peas_extension_python_parent_class)->finalize (object);
}

static void
peas_extension_python_class_init (PeasExtensionPythonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasExtensionClass *extension_class = PEAS_EXTENSION_CLASS (klass);

  object_class->finalize = peas_extension_python_finalize;

  extension_class->call = peas_extension_python_call;
}

PeasExtension *
peas_extension_python_new (GType     gtype,
                           PyObject *instance)
{
  PeasExtensionPython *pyexten;
  GType real_type;

  real_type = peas_extension_register_subclass (PEAS_TYPE_EXTENSION_PYTHON, gtype);
  pyexten = PEAS_EXTENSION_PYTHON (g_object_new (real_type,
                                                 "extension-type", gtype,
                                                 NULL));
  pyexten->instance = instance;
  Py_INCREF (instance);

  return PEAS_EXTENSION (pyexten);
}
