/*
 * peas-extension-c.h
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

#ifndef __PEAS_EXTENSION_C_H__
#define __PEAS_EXTENSION_C_H__

#include <libpeas/peas-extension-priv.h>

G_BEGIN_DECLS

#define PEAS_TYPE_EXTENSION_C            (peas_extension_c_get_type ())
#define PEAS_EXTENSION_C(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_EXTENSION_C, PeasExtensionC))
#define PEAS_EXTENSION_C_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PEAS_TYPE_EXTENSION_C, PeasExtensionCClass))
#define PEAS_IS_EXTENSION_C(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_EXTENSION_C))
#define PEAS_IS_EXTENSION_C_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_EXTENSION_C))
#define PEAS_EXTENSION_C_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PEAS_TYPE_EXTENSION_C, PeasExtensionCClass))

typedef struct _PeasExtensionC       PeasExtensionC;
typedef struct _PeasExtensionCClass  PeasExtensionCClass;

struct _PeasExtensionC {
  PeasExtension parent;

  GObject *instance;
};

struct _PeasExtensionCClass {
  PeasExtensionClass parent_class;
};

GType            peas_extension_c_get_type  (void) G_GNUC_CONST;

PeasExtension   *peas_extension_c_new       (GType        gtype,
                                             GObject     *instance);

G_END_DECLS

#endif /* __PEAS_PLUGIN_LOADER_C_H__ */
