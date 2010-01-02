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

#include "config.h"

void pypeas_register_classes (PyObject *d);
extern PyMethodDef pypeas_functions[];

DL_EXPORT(void)
initlibpeas (void)
{
  PyObject *m, *d;
  PyObject *tuple;

  m = Py_InitModule ("libpeas", pypeas_functions);
  d = PyModule_GetDict (m);

  init_pygobject_check (2, 16, 2);

  pypeas_register_classes (d);

  /* libpeas version */
  tuple = Py_BuildValue ("(iii)",
                         PEAS_MAJOR_VERSION,
                         PEAS_MINOR_VERSION,
                         PEAS_MICRO_VERSION);
  PyDict_SetItemString (d, "version", tuple); 
  Py_DECREF (tuple);
}
