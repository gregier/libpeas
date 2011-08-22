/*
 * peas-plugin-store-backend-pk.h
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _PEAS_GTK_GTK_PLUGIN_STORE_BACKEND_PK_H_
#define _PEAS_GTK_GTK_PLUGIN_STORE_BACKEND_PK_H_

#include <libpeas/peas-engine.h>

#include "peas-gtk-plugin-store-backend.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_PK            (peas_gtk_plugin_store_backend_pk_get_type())
#define PEAS_GTK_PLUGIN_STORE_BACKEND_PK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_PK, PeasGtkPluginStoreBackendPk))
#define PEAS_GTK_PLUGIN_STORE_BACKEND_PK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_PK, PeasGtkPluginStoreBackendPkClass))
#define PEAS_GTK_IS_PLUGIN_STORE_BACKEND_PK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_PK))
#define PEAS_GTK_IS_PLUGIN_STORE_BACKEND_PK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_PK))
#define PEAS_GTK_PLUGIN_STORE_BACKEND_PK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_GTK_TYPE_PLUGIN_STORE_BACKEND_PK, PeasGtkPluginStoreBackendPkClass))

typedef struct _PeasGtkPluginStoreBackendPk        PeasGtkPluginStoreBackendPk;
typedef struct _PeasGtkPluginStoreBackendPkClass   PeasGtkPluginStoreBackendPkClass;
typedef struct _PeasGtkPluginStoreBackendPkPrivate PeasGtkPluginStoreBackendPkPrivate;

struct _PeasGtkPluginStoreBackendPk
{
  GObject parent;

  PeasGtkPluginStoreBackendPkPrivate *priv;
};

struct _PeasGtkPluginStoreBackendPkClass
{
  GObjectClass parent_class;
};

GType peas_gtk_plugin_store_backend_pk_get_type (void) G_GNUC_CONST;

PeasGtkPluginStoreBackend *
      peas_gtk_plugin_store_backend_pk_new      (PeasEngine *engine);

G_END_DECLS

#endif /* _PEAS_GTK_GTK_PLUGIN_STORE_BACKEND_PK_H_  */
