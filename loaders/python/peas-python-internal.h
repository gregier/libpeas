/*
 * peas-python-internal.h
 * This file is part of libpeas
 *
 * Copyright (C) 2015 - Garrett Regier
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

#ifndef __PEAS_PYTHON_INTERNAL_H__
#define __PEAS_PYTHON_INTERNAL_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _PeasPythonInternal PeasPythonInternal;

PeasPythonInternal *
        peas_python_internal_new  (void);
void    peas_python_internal_free (PeasPythonInternal *internal);

void    peas_python_internal_call (PeasPythonInternal *internal,
                                   const gchar        *name);

G_END_DECLS

#endif /* __PEAS_PYTHON_INTERNAL_H__ */
