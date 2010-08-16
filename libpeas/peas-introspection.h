/*
 * peas-introspection.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
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

#ifndef __PEAS_INTROSPECTION_H__
#define __PEAS_INTROSPECTION_H__

#include <glib-object.h>
#include <girepository.h>

G_BEGIN_DECLS

GICallableInfo  *peas_gi_get_method_info          (GType        iface_type,
                                                   const gchar *method_name);

void             peas_gi_valist_to_arguments      (GICallableInfo *callable_info,
                                                   va_list         va_args,
                                                   GArgument      *arguments,
                                                   gpointer       *return_value);
void             peas_gi_argument_to_pointer      (GITypeInfo     *type_info,
                                                   GArgument      *arg,
                                                   gpointer        ptr);
gboolean         peas_method_apply                (GObject     *instance,
                                                   GType        iface_type,
                                                   const gchar *method_name,
                                                   GArgument   *args,
                                                   GArgument   *return_value);

G_END_DECLS

#endif
