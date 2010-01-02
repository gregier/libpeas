/*
 * libpeasmodule.c
 * This file is part of libpeas
 *
 * Copyright (C) 2009 - Steve Frécinaux
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include <Python.h>
#include <pyglib.h>
#include <pygobject.h>
#include <pygtk/pygtk.h>

#include "config.h"

void pypeasui_register_classes (PyObject *d);
extern PyMethodDef pypeasui_functions[];

static PyObject *
build_version_tuple (void)
{
  return Py_BuildValue ("(iii)",
                        PEAS_MAJOR_VERSION,
                        PEAS_MINOR_VERSION,
                        PEAS_MICRO_VERSION);
}



/* C equivalent of
 *    import libpeas
 *    assert(libpeas.version > (MAJ, MIN, MIC))
 */
static gboolean
pypeasui_init_libpeas (void)
{
  PyObject *libpeas, *mdict, *libpeas_version, *required_version;

  libpeas = PyImport_ImportModule ("libpeas");
  if (libpeas == NULL)
    {
      PyErr_SetString (PyExc_ImportError, "Could not import libpeas.");
      return FALSE;
    }

  mdict = PyModule_GetDict (libpeas);
  libpeas_version = PyDict_GetItemString (mdict, "version");
  if (!libpeas_version)
    {
      PyErr_SetString (PyExc_ImportError, "Could not retrieve libpeas version.");
      return;
    }

  required_version = build_version_tuple ();
  if (PyObject_Compare (libpeas_version, required_version) < 0)
    PyErr_SetString (PyExc_ImportError, "libpeas version is too old.");

  Py_DECREF (required_version);
}

DL_EXPORT(void)
initlibpeasui (void)
{
  PyObject *m, *d;
  PyObject *tuple;

  m = Py_InitModule ("libpeasui", pypeasui_functions);
  d = PyModule_GetDict (m);

  init_pygobject_check (2, 16, 2);
  init_pygtk ();

  pypeasui_init_libpeas ();
  if (PyErr_Occurred ())
    return;

  pypeasui_register_classes (d);

  /* libpeasui version */
  tuple = build_version_tuple ();
  PyDict_SetItemString (d, "version", tuple);
  Py_DECREF (tuple);
}
