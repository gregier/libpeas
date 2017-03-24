/*
 * extension-c-abstract.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>

#include <libpeas/peas.h>

#include "extension-c-abstract.h"

G_DEFINE_DYNAMIC_TYPE (TestingExtensionCAbstract,
                       testing_extension_c_abstract,
                       INTROSPECTION_TYPE_ABSTRACT)

static void
testing_extension_c_abstract_init (TestingExtensionCAbstract *abstract)
{
}

static void
testing_extension_c_abstract_class_init (TestingExtensionCAbstractClass *klass)
{
}

static void
testing_extension_c_abstract_class_finalize (TestingExtensionCAbstractClass *klass)
{
}

void
testing_extension_c_abstract_register (GTypeModule *module)
{
  testing_extension_c_abstract_register_type (module);
}
