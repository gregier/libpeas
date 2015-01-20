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


typedef PyObject _PeasPythonInternal;

static PyObject *FailedError = NULL;


static PyObject *
failed_fn (PyObject *self,
           PyObject *args)
{
  const gchar *msg;
  gchar *clean_msg;

  if (!PyArg_ParseTuple (args, "s:Hooks.failed", &msg))
    return NULL;

  /* Python tracebacks have a trailing newline */
  clean_msg = g_strchomp (g_strdup (msg));

  g_warning ("%s", clean_msg);

  /* peas_python_internal_call() knows to check for this exception */
  PyErr_SetObject (FailedError, NULL);

  g_free (clean_msg);
  return NULL;
}

static PyMethodDef failed_method_def = {
  "failed", (PyCFunction) failed_fn, METH_VARARGS | METH_STATIC,
  "Prints warning and raises an Exception"
};

PeasPythonInternal *
peas_python_internal_new (gboolean already_initialized)
{
  GBytes *internal_python;
  const gchar *prgname;
  PeasPythonInternal *internal = NULL;
  PyObject *builtins_module, *globals;
  PyObject *code = NULL, *module = NULL;
  PyObject *result = NULL, *failed_method = NULL;

#define goto_error_if_failed(cond) \
  G_STMT_START { \
    if (G_UNLIKELY (!(cond))) \
      { \
        g_warn_if_fail (cond); \
        goto error; \
      } \
  } G_STMT_END

  prgname = g_get_prgname ();
  prgname = prgname == NULL ? "" : prgname;

#if PY_MAJOR_VERSION < 3
  builtins_module = PyImport_ImportModule ("__builtin__");
#else
  builtins_module = PyImport_ImportModule ("builtins");
#endif

  goto_error_if_failed (builtins_module != NULL);

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
  goto_error_if_failed (internal_python != NULL);

  /* Compile it manually so the filename is available */
  code = Py_CompileString (g_bytes_get_data (internal_python, NULL),
                           "peas-python-internal.py",
                           Py_file_input);
  goto_error_if_failed (code != NULL);

  module = PyModule_New ("libpeas-internal");
  goto_error_if_failed (module != NULL);

  goto_error_if_failed (PyModule_AddObject (module, "__builtins__",
                                            builtins_module) == 0);
  goto_error_if_failed (PyModule_AddObject (module, "ALREADY_INITIALIZED",
                                            already_initialized ?
                                            Py_True : Py_False) == 0);
  goto_error_if_failed (PyModule_AddStringConstant (module, "PRGNAME",
                                                    prgname) == 0);
  goto_error_if_failed (PyModule_AddStringMacro (module,
                                                 PEAS_PYEXECDIR) == 0);
  goto_error_if_failed (PyModule_AddStringMacro (module,
                                                 GETTEXT_PACKAGE) == 0);
  goto_error_if_failed (PyModule_AddStringMacro (module,
                                                 PEAS_LOCALEDIR) == 0);

  globals = PyModule_GetDict (module);
  result = PyEval_EvalCode ((gpointer) code, globals, globals);
  Py_XDECREF (result);

  if (PyErr_Occurred ())
    {
      g_warning ("Failed to run internal Python code");
      goto error;
    }

  result = PyDict_GetItemString (globals, "hooks");
  goto_error_if_failed (result != NULL);

  goto_error_if_failed (PyObject_SetAttrString (result,
                                                "__internal_module__",
                                                module) == 0);

  FailedError = PyDict_GetItemString (globals, "FailedError");
  goto_error_if_failed (FailedError != NULL);

  failed_method = PyCFunction_NewEx (&failed_method_def, NULL, module);
  goto_error_if_failed (failed_method != NULL);
  goto_error_if_failed (PyObject_SetAttrString (result, "failed",
                                                failed_method) == 0);

  internal = (PeasPythonInternal *) result;

#undef goto_error_if_failed

error:

  if (internal == NULL)
    Py_XDECREF (result);

  Py_XDECREF (failed_method);
  Py_XDECREF (module);
  Py_XDECREF (code);
  g_clear_pointer (&internal_python, g_bytes_unref);

  return internal;
}

void
peas_python_internal_free (PeasPythonInternal *internal)
{
  PyObject *internal_ = (PyObject *) internal;

  peas_python_internal_call (internal, "exit", NULL, NULL);
  Py_DECREF (internal_);
}

PyObject *
peas_python_internal_call (PeasPythonInternal *internal,
                           const gchar        *name,
                           PyTypeObject       *return_type,
                           const gchar        *format,
                           ...)
{
  PyObject *internal_ = (PyObject *) internal;
  PyObject *args;
  PyObject *result = NULL;
  va_list var_args;

  /* The PyTypeObject for Py_None is not exposed directly */
  if (return_type == NULL)
    return_type = Py_None->ob_type;

  va_start (var_args, format);
  args = Py_VaBuildValue (format == NULL ? "()" : format, var_args);
  va_end (var_args);

  if (args != NULL)
    {
      result = PyObject_CallMethod (internal_, "call", "(sOO)",
                                    name, args, return_type);
      Py_DECREF (args);
    }

  if (PyErr_Occurred ())
    {
      /* Raised by failed_fn() to prevent printing the exception */
      if (PyErr_ExceptionMatches (FailedError))
        {
          PyErr_Clear ();
        }
      else
        {
          g_warning ("Failed to run internal Python hook '%s'", name);
          PyErr_Print ();
        }

      return NULL;
    }

  if (result == Py_None)
    Py_CLEAR (result);

  return result;
}
