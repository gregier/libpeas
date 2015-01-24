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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "introspection-base.h"

G_DEFINE_INTERFACE(IntrospectionBase, introspection_base, G_TYPE_OBJECT)

void
introspection_base_default_init (IntrospectionBaseInterface *iface)
{
}

/**
 * introspection_base_get_plugin_info:
 * @base:
 *
 * Returns: (transfer none):
 */
const PeasPluginInfo *
introspection_base_get_plugin_info (IntrospectionBase *base)
{
  IntrospectionBaseInterface *iface;

  g_return_val_if_fail (INTROSPECTION_IS_BASE (base), NULL);

  iface = INTROSPECTION_BASE_GET_IFACE (base);
  g_assert (iface->get_plugin_info != NULL);

  return iface->get_plugin_info (base);
}

/**
 * introspection_base_get_settings:
 * @base:
 *
 * Returns: (transfer full):
 */
GSettings *
introspection_base_get_settings (IntrospectionBase *base)
{
  IntrospectionBaseInterface *iface;

  g_return_val_if_fail (INTROSPECTION_IS_BASE (base), NULL);

  iface = INTROSPECTION_BASE_GET_IFACE (base);
  g_assert (iface->get_settings != NULL);

  return iface->get_settings (base);
}
