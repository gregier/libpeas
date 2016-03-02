/*
 * introspection-prerequisite.h
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

#ifndef __INTROSPECTION_PREREQUISITE_H__
#define __INTROSPECTION_PREREQUISITE_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define INTROSPECTION_TYPE_PREREQUISITE           (introspection_prerequisite_get_type ())
#define INTROSPECTION_PREREQUISITE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), INTROSPECTION_TYPE_PREREQUISITE, IntrospectionPrerequisite))
#define INTROSPECTION_PREREQUISITE_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), INTROSPECTION_TYPE_PREREQUISITE, IntrospectionPrerequisiteClass))
#define INTROSPECTION_IS_PREREQUISITE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INTROSPECTION_TYPE_PREREQUISITE))
#define INTROSPECTION_PREREQUISITE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), INTROSPECTION_TYPE_PREREQUISITE, IntrospectionPrerequisiteClass))

typedef struct _IntrospectionPrerequisite         IntrospectionPrerequisite;
typedef struct _IntrospectionPrerequisiteClass    IntrospectionPrerequisiteClass;

struct _IntrospectionPrerequisite {
  PeasExtensionBase parent;
};

struct _IntrospectionPrerequisiteClass {
  PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType        introspection_prerequisite_get_type         (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __INTROSPECTION_PREREQUISITE_H__ */
