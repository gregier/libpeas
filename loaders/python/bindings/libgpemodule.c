/*
 * libgpemodule.c
 * This file is part of libgpe
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

void pygpe_register_classes (PyObject *d);
extern PyMethodDef pygpe_functions[];

DL_EXPORT(void)
initlibgpe (void)
{
    PyObject *m, *d;
    PyObject *tuple;

    m = Py_InitModule ("libgpe", pygpe_functions);
    d = PyModule_GetDict (m);

    init_pygobject_check (2, 16, 2);

    pygpe_register_classes (d);

    /* pygio version */
    tuple = Py_BuildValue ("(iii)",
			   GPE_MAJOR_VERSION,
			   GPE_MINOR_VERSION,
			   GPE_MICRO_VERSION);
    PyDict_SetItemString (d, "version", tuple); 
    Py_DECREF (tuple);
}
