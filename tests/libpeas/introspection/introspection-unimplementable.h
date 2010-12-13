/*
 * introspection-unimplementable.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Garrett Regier
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

#ifndef __INTROSPECTION_UNIMPLEMENTABLE_H__
#define __INTROSPECTION_UNIMPLEMENTABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define INTROSPECTION_TYPE_UNIMPLEMENTABLE             (introspection_unimplementable_get_type ())
#define INTROSPECTION_UNIMPLEMENTABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), INTROSPECTION_TYPE_UNIMPLEMENTABLE, IntrospectionUnimplementable))
#define INTROSPECTION_UNIMPLEMENTABLE_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), INTROSPECTION_TYPE_UNIMPLEMENTABLE, IntrospectionUnimplementableInterface))
#define INTROSPECTION_IS_UNIMPLEMENTABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INTROSPECTION_TYPE_UNIMPLEMENTABLE))
#define INTROSPECTION_UNIMPLEMENTABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), INTROSPECTION_TYPE_UNIMPLEMENTABLE, IntrospectionUnimplementableInterface))

typedef struct _IntrospectionUnimplementable           IntrospectionUnimplementable; /* dummy typedef */
typedef struct _IntrospectionUnimplementableInterface  IntrospectionUnimplementableInterface;

struct _IntrospectionUnimplementableInterface {
  GTypeInterface g_iface;
};

/*
 * Public methods
 */
GType introspection_unimplementable_get_type (void)  G_GNUC_CONST;

G_END_DECLS

#endif /* __INTROSPECTION_UNIMPLEMENTABLE_H__ */
