/*
 * introspection-has-prerequisite.h
 * This file is part of libpeas
 *
 * Copyright (C) 2011-2017 Garrett Regier
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

#include "introspection-base.h"
#include "introspection-callable.h"
#include "introspection-has-prerequisite.h"

#include "introspection-prerequisite.h"

G_DEFINE_INTERFACE_WITH_CODE(IntrospectionHasPrerequisite,
                             introspection_has_prerequisite,
                             INTROSPECTION_TYPE_PREREQUISITE,
                             g_type_interface_add_prerequisite (g_define_type_id,
                                                                INTROSPECTION_TYPE_BASE);
                             g_type_interface_add_prerequisite (g_define_type_id,
                                                                INTROSPECTION_TYPE_CALLABLE);)

void
introspection_has_prerequisite_default_init (IntrospectionHasPrerequisiteInterface *iface)
{
}
