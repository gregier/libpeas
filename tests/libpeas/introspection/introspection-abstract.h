/*
 * introspection-abstract.h
 * This file is part of libpeas
 *
 * Copyright (C) 2017 Garrett Regier
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

#ifndef __INTROSPECTION_ABSTRACT_H__
#define __INTROSPECTION_ABSTRACT_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define INTROSPECTION_TYPE_ABSTRACT           (introspection_abstract_get_type ())
#define INTROSPECTION_ABSTRACT(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), INTROSPECTION_TYPE_ABSTRACT, IntrospectionAbstract))
#define INTROSPECTION_ABSTRACT_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), INTROSPECTION_TYPE_ABSTRACT, IntrospectionAbstractClass))
#define INTROSPECTION_IS_ABSTRACT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INTROSPECTION_TYPE_ABSTRACT))
#define INTROSPECTION_ABSTRACT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), INTROSPECTION_TYPE_ABSTRACT, IntrospectionAbstractClass))

typedef struct _IntrospectionAbstract         IntrospectionAbstract;
typedef struct _IntrospectionAbstractClass    IntrospectionAbstractClass;

struct _IntrospectionAbstract {
  PeasExtensionBase parent;
};

struct _IntrospectionAbstractClass {
  PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType  introspection_abstract_get_type  (void) G_GNUC_CONST;

gint   introspection_abstract_get_value (IntrospectionAbstract *abstract);
void   introspection_abstract_set_value (IntrospectionAbstract *abstract,
                                         gint                   value);

G_END_DECLS

#endif /* __INTROSPECTION_ABSTRACT_H__ */
