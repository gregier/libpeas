/*
 * introspection-callable.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Garrett Regier
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "introspection-callable.h"

G_DEFINE_INTERFACE(IntrospectionCallable, introspection_callable, G_TYPE_OBJECT)

void
introspection_callable_default_init (IntrospectionCallableInterface *iface)
{
}

/**
 * introspection_callable_call_with_return:
 * callable:
 *
 * Returns: (transfer full):
 */
const gchar *
introspection_callable_call_with_return (IntrospectionCallable *callable)
{
  IntrospectionCallableInterface *iface;

  g_return_val_if_fail (INTROSPECTION_IS_CALLABLE (callable), NULL);

  iface = INTROSPECTION_CALLABLE_GET_IFACE (callable);
  if (iface->call_with_return != NULL)
    return iface->call_with_return (callable);

  return NULL;
}

/**
 * introspection_callable_call_single_arg:
 * callable:
 * called: (out):
 */
void
introspection_callable_call_single_arg (IntrospectionCallable *callable,
                                        gboolean              *called)
{
  IntrospectionCallableInterface *iface;

  g_return_if_fail (INTROSPECTION_IS_CALLABLE (callable));

  iface = INTROSPECTION_CALLABLE_GET_IFACE (callable);
  if (iface->call_single_arg != NULL)
    iface->call_single_arg (callable, called);
}

/**
 * introspection_callable_call_multi_args:
 * callable:
 * called_1: (out):
 * called_2: (out):
 * called_3: (out):
 */
void
introspection_callable_call_multi_args (IntrospectionCallable *callable,
                                        gboolean              *called_1,
                                        gboolean              *called_2,
                                        gboolean              *called_3)
{
  IntrospectionCallableInterface *iface;

  g_return_if_fail (INTROSPECTION_IS_CALLABLE (callable));

  iface = INTROSPECTION_CALLABLE_GET_IFACE (callable);
  if (iface->call_multi_args != NULL)
    iface->call_multi_args (callable, called_1, called_2, called_3);
}
