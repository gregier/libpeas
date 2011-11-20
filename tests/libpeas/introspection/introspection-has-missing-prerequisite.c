/*
 * introspection-has-prerequisite.h
 * This file is part of libpeas
 *
 * Copyright (C) 2011 Garrett Regier
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "introspection-has-missing-prerequisite.h"

#include "introspection-callable.h"

G_DEFINE_INTERFACE(IntrospectionHasMissingPrerequisite,
                   introspection_has_missing_prerequisite,
                   INTROSPECTION_TYPE_CALLABLE)

void
introspection_has_missing_prerequisite_default_init (IntrospectionHasMissingPrerequisiteInterface *iface)
{
}
