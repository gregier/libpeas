/*
 * extension-c-abstract.h
 * This file is part of libpeas
 *
 * Copyright (C) 2017 - Garrett Regier
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

#ifndef __EXTENSION_C_ABSTRACT_H__
#define __EXTENSION_C_ABSTRACT_H__

#include "introspection-abstract.h"

G_BEGIN_DECLS

#define TESTING_TYPE_EXTENSION_C_ABSTRACT         (testing_extension_c_abstract_get_type ())
#define TESTING_EXTENSION_C_ABSTRACT(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_EXTENSION_C_ABSTRACT, TestingExtensionCAbstract))
#define TESTING_EXTENSION_C_ABSTRACT_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_EXTENSION_C_ABSTRACT, TestingExtensionCAbstract))
#define TESTING_IS_EXTENSION_C_ABSTRACT(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_EXTENSION_C_ABSTRACT))
#define TESTING_IS_EXTENSION_C_ABSTRACT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_EXTENSION_C_ABSTRACT))
#define TESTING_EXTENSION_C_ABSTRACT_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_EXTENSION_C_ABSTRACT, TestingExtensionCAbstractClass))

typedef struct _TestingExtensionCAbstract         TestingExtensionCAbstract;
typedef struct _TestingExtensionCAbstractClass    TestingExtensionCAbstractClass;

struct _TestingExtensionCAbstract {
  IntrospectionAbstract parent_instance;
};

struct _TestingExtensionCAbstractClass {
  IntrospectionAbstractClass parent_class;
};

GType testing_extension_c_abstract_get_type (void) G_GNUC_CONST;
void  testing_extension_c_abstract_register (GTypeModule *module);

G_END_DECLS

#endif /* __EXTENSION_C_ABSTRACT_H__ */
