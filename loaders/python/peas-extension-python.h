/*
 * peas-extension-python.h
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

#ifndef __PEAS_EXTENSION_PYTHON_H__
#define __PEAS_EXTENSION_PYTHON_H__

#include <libpeas/peas-extension-priv.h>
/* _POSIX_C_SOURCE is defined in Python.h and in limits.h included by
 * <libpeas/peas-extension.h>, so we unset it here to avoid a warning.
 * Yep, that's bad. */
#undef _POSIX_C_SOURCE
#include <Python.h>

G_BEGIN_DECLS

#define PEAS_TYPE_EXTENSION_PYTHON            (peas_extension_python_get_type ())
#define PEAS_EXTENSION_PYTHON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_EXTENSION_PYTHON, PeasExtensionPython))
#define PEAS_EXTENSION_PYTHON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PEAS_TYPE_EXTENSION_PYTHON, PeasExtensionPythonClass))
#define PEAS_IS_EXTENSION_PYTHON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_EXTENSION_PYTHON))
#define PEAS_IS_EXTENSION_PYTHON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_EXTENSION_PYTHON))
#define PEAS_EXTENSION_PYTHON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PEAS_TYPE_EXTENSION_PYTHON, PeasExtensionPythonClass))

typedef struct _PeasExtensionPython       PeasExtensionPython;
typedef struct _PeasExtensionPythonClass  PeasExtensionPythonClass;

struct _PeasExtensionPython {
  PeasExtension parent;

  PyObject *instance;
};

struct _PeasExtensionPythonClass {
  PeasExtensionClass parent_class;
};

GType            peas_extension_python_get_type (void) G_GNUC_CONST;

PeasExtension   *peas_extension_python_new      (GType        gtype,
                                                 PyObject    *instance);

G_END_DECLS

#endif /* __PEAS_EXTENSION_PYTHON_H__ */
