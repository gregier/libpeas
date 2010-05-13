/*
 * peas-extension.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
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

#include "peas-extension.h"

/**
 * SECTION:peas-extension
 * @short_description: Proxy for extensions.
 *
 * A #PeasExtension is an object which proxies an actual extension.
 **/

G_DEFINE_ABSTRACT_TYPE (PeasExtension, peas_extension, G_TYPE_OBJECT);

static void
peas_extension_init (PeasExtension *exten)
{
}

static void
peas_extension_class_init (PeasExtensionClass *klass)
{
}

/**
 * peas_extension_call:
 * @exten: A #PeasExtension.
 * @method_name: the name of the method that should be called.
 *
 * Call a method of the object behind @extension.
 *
 * Return value: #TRUE on successful call.
 */
gboolean
peas_extension_call (PeasExtension *exten,
                     const gchar   *method_name,
                     ...)
{
  va_list args;
  gboolean result;

  va_start (args, method_name);
  result = peas_extension_call_valist (exten, method_name, args);
  va_end (args);

  return result;
}

gboolean
peas_extension_call_valist (PeasExtension *exten,
                            const gchar   *method_name,
                            va_list args)
{
  PeasExtensionClass *klass;

  g_return_val_if_fail (PEAS_IS_EXTENSION (exten), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  klass = PEAS_EXTENSION_GET_CLASS (exten);
  return klass->call (exten, method_name, args);
}
