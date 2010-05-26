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

#include "config.h"
#include  <girepository.h>
#include <libpeas/peas-introspection.h>
#include "peas-extension-c.h"

G_DEFINE_DYNAMIC_TYPE (PeasExtensionC, peas_extension_c, PEAS_TYPE_EXTENSION);

void
peas_extension_c_register (GTypeModule *module)
{
  peas_extension_c_register_type (module);
}

static void
peas_extension_c_init (PeasExtensionC *cexten)
{
}

static gboolean
peas_extension_c_call (PeasExtension *exten,
                       const gchar   *method_name,
                       va_list        args)
{
  PeasExtensionC *cexten = PEAS_EXTENSION_C (exten);

  return peas_method_apply_valist (cexten->instance, cexten->gtype, method_name, args);
}

static void
peas_extension_c_finalize (GObject *object)
{
  PeasExtensionC *cexten = PEAS_EXTENSION_C (object);
  
  if (cexten->instance)
    g_object_unref (cexten->instance);

  G_OBJECT_CLASS (peas_extension_c_parent_class)->finalize (object);
}

static void
peas_extension_c_class_init (PeasExtensionCClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasExtensionClass *extension_class = PEAS_EXTENSION_CLASS (klass);

  object_class->finalize = peas_extension_c_finalize;

  extension_class->call = peas_extension_c_call;
}

static void
peas_extension_c_class_finalize (PeasExtensionCClass *klass)
{
}

PeasExtension *
peas_extension_c_new (GType    gtype,
                      GObject *instance)
{
  PeasExtensionC *cexten;

  cexten = PEAS_EXTENSION_C (g_object_new (PEAS_TYPE_EXTENSION_C, NULL));
  cexten->gtype = gtype;
  cexten->instance = instance;

  return PEAS_EXTENSION (cexten);
}
