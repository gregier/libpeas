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

#include "config.h"
#include <girepository.h>
/* _POSIX_C_SOURCE is defined in Python.h and in limits.h included by
 * girepository.h, so we unset it here to avoid a warning. Yep, that's bad. */
#undef _POSIX_C_SOURCE
#include <Python.h>
#include <pygobject.h>
#include <libpeas/peas-introspection.h>
#include "peas-extension-python.h"

G_DEFINE_DYNAMIC_TYPE (PeasExtensionPython, peas_extension_python, PEAS_TYPE_EXTENSION);

void
peas_extension_python_register (GTypeModule *module)
{
  peas_extension_python_register_type (module);
}

static void
peas_extension_python_init (PeasExtensionPython *pyexten)
{
}

static gboolean
peas_extension_python_call (PeasExtension *exten,
                            const gchar   *method_name,
                            va_list        args)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (exten);
  GObject *instance;

  instance = pygobject_get (pyexten->instance);

  return peas_method_apply_valist (instance, pyexten->gtype, method_name, args);
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

static void
peas_extension_python_class_finalize (PeasExtensionPythonClass *klass)
{
}

PeasExtension *
peas_extension_python_new (GType     gtype,
                           PyObject *instance)
{
  PeasExtensionPython *pyexten;

  pyexten = PEAS_EXTENSION_PYTHON (g_object_new (PEAS_TYPE_EXTENSION_PYTHON, NULL));
  pyexten->gtype = gtype;
  pyexten->instance = instance;
  Py_INCREF (instance);

  return PEAS_EXTENSION (pyexten);
}
