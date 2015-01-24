/*
 * introspection-base.h
 * This file is part of libpeas
 *
 * Copyright (C) 2011 - Garrett Regier
 *
 * libpeas is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libpeas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifndef __INTROSPECTION_BASE_H__
#define __INTROSPECTION_BASE_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define INTROSPECTION_TYPE_BASE             (introspection_base_get_type ())
#define INTROSPECTION_BASE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), INTROSPECTION_TYPE_BASE, IntrospectionBase))
#define INTROSPECTION_BASE_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), INTROSPECTION_TYPE_BASE, IntrospectionBaseInterface))
#define INTROSPECTION_IS_BASE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INTROSPECTION_TYPE_BASE))
#define INTROSPECTION_BASE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), INTROSPECTION_TYPE_BASE, IntrospectionBaseInterface))

typedef struct _IntrospectionBase           IntrospectionBase; /* dummy typedef */
typedef struct _IntrospectionBaseInterface  IntrospectionBaseInterface;

struct _IntrospectionBaseInterface {
  GTypeInterface g_iface;

  /* Virtual public methods */
  const PeasPluginInfo *(*get_plugin_info) (IntrospectionBase *base);
  GSettings            *(*get_settings)    (IntrospectionBase *base);
};

/*
 * Public methods
 */
GType      introspection_base_get_type     (void) G_GNUC_CONST;

const PeasPluginInfo *introspection_base_get_plugin_info (IntrospectionBase *base);
GSettings            *introspection_base_get_settings    (IntrospectionBase *base);

G_END_DECLS

#endif /* __INTROSPECTION_BASE_H__ */
