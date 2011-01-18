/*
 * peas-extension-c.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Steve Frécinaux
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

#include  <girepository.h>
#include <libpeas/peas-introspection.h>
#include <libpeas/peas-extension-subclasses.h>
#include "peas-extension-c.h"

G_DEFINE_TYPE (PeasExtensionC, peas_extension_c, PEAS_TYPE_EXTENSION);

static void
peas_extension_c_init (PeasExtensionC *cexten)
{
}

static gboolean
peas_extension_c_call (PeasExtension *exten,
                       GType          gtype,
                       const gchar   *method_name,
                       GIArgument    *args,
                       GIArgument    *retval)
{
  PeasExtensionC *cexten = PEAS_EXTENSION_C (exten);

  if (gtype == G_TYPE_INVALID)
    gtype = peas_extension_get_extension_type (exten);

  return peas_method_apply (cexten->instance, gtype, method_name, args, retval);
}

static void
peas_extension_c_dispose (GObject *object)
{
  PeasExtensionC *cexten = PEAS_EXTENSION_C (object);
  
  if (cexten->instance)
    {
      g_object_unref (cexten->instance);
      cexten->instance = NULL;
    }

  G_OBJECT_CLASS (peas_extension_c_parent_class)->dispose (object);
}

static void
peas_extension_c_class_init (PeasExtensionCClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasExtensionClass *extension_class = PEAS_EXTENSION_CLASS (klass);

  object_class->dispose = peas_extension_c_dispose;

  extension_class->call = peas_extension_c_call;
}

PeasExtension *
peas_extension_c_new (GType    gtype,
                      GObject *instance)
{
  PeasExtensionC *cexten;
  GType real_type;

  real_type = peas_extension_register_subclass (PEAS_TYPE_EXTENSION_C, gtype);
  cexten = PEAS_EXTENSION_C (g_object_new (real_type,
                                           "extension-type", gtype,
                                           NULL));
  cexten->instance = instance;

  return PEAS_EXTENSION (cexten);
}
