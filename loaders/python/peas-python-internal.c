/*
 * peas-python-internal.c
 * This file is part of libpeas
 *
 * Copyright (C) 2014-2015 - Garrett Regier
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

#include "peas-python-internal.h"

#include <gio/gio.h>

/* _POSIX_C_SOURCE is defined in Python.h and in limits.h included by
 * glib-object.h, so we unset it here to avoid a warning. Yep, that's bad.
 */
#undef _POSIX_C_SOURCE
#include <Python.h>


typedef PyObject _PeasPythonInternal;


PeasPythonInternal *
peas_python_internal_new (void)
{
  GBytes *internal_python;
  PyObject *builtins_module, *code, *globals, *result, *internal;

#if PY_MAJOR_VERSION < 3
  builtins_module = PyImport_ImportModule ("__builtin__");
#else
  builtins_module = PyImport_ImportModule ("builtins");
#endif

  if (builtins_module == NULL)
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "failed to get builtins module");

      return NULL;
    }

  /* We don't use the byte-compiled Python source
   * because glib-compile-resources cannot output
   * depends for generated files.
   *
   * https://bugzilla.gnome.org/show_bug.cgi?id=673101
   */

  internal_python = g_resources_lookup_data ("/org/gnome/libpeas/loaders/"
#if PY_MAJOR_VERSION < 3
                                             "python/"
#else
                                             "python3/"
#endif
                                             "internal.py",
                                             G_RESOURCE_LOOKUP_FLAGS_NONE,
                                             NULL);

  if (internal_python == NULL)
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "failed to locate internal Python code");

      return NULL;
    }

  /* Compile it manually so the filename is available */
  code = Py_CompileString (g_bytes_get_data (internal_python, NULL),
                           "peas-python-internal.py",
                           Py_file_input);

  g_bytes_unref (internal_python);

  if (code == NULL)
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "failed to compile internal Python code");

      return NULL;
    }

  globals = PyDict_New ();
  if (globals == NULL)
    {
      Py_DECREF (code);
      return NULL;
    }

  if (PyDict_SetItemString (globals, "__builtins__",
                            PyModule_GetDict (builtins_module)) != 0)
    {
      Py_DECREF (globals);
      Py_DECREF (code);
      return NULL;
    }

  result = PyEval_EvalCode ((gpointer) code, globals, globals);
  Py_XDECREF (result);

  Py_DECREF (code);

  if (PyErr_Occurred ())
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "failed to run internal Python code");

      Py_DECREF (globals);
      return NULL;
    }

  internal = PyDict_GetItemString (globals, "hooks");
  Py_XINCREF (internal);

  Py_DECREF (globals);

  if (internal == NULL)
    {
      g_warning ("Error initializing Python Plugin Loader: "
                 "failed to find internal Python hooks");

      return NULL;
    }

  return (PeasPythonInternal *) internal;
}

/* NOTE: This must be called with the GIL held */
void
peas_python_internal_free (PeasPythonInternal *internal)
{
  PyObject *internal_ = (PyObject *) internal;

  peas_python_internal_call (internal, "exit");
  Py_DECREF (internal_);
}

/* NOTE: This must be called with the GIL held */
void
peas_python_internal_call (PeasPythonInternal *internal,
                           const gchar        *name)
{
  PyObject *internal_ = (PyObject *) internal;
  PyObject *result;

  result = PyObject_CallMethod (internal_, (gchar *) name, NULL);
  Py_XDECREF (result);

  if (PyErr_Occurred ())
    PyErr_Print ();
}

