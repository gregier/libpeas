/*
 * peas-plugin-loader-c.h
 * This file is part of libpeas
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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

#ifndef __PEAS_PLUGIN_LOADER_C_H__
#define __PEAS_PLUGIN_LOADER_C_H__

#include "peas-plugin-loader.h"

G_BEGIN_DECLS

#define PEAS_TYPE_PLUGIN_LOADER_C            (peas_plugin_loader_c_get_type ())
#define PEAS_PLUGIN_LOADER_C(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_PLUGIN_LOADER_C, PeasPluginLoaderC))
#define PEAS_PLUGIN_LOADER_C_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PEAS_TYPE_PLUGIN_LOADER_C, PeasPluginLoaderCClass))
#define PEAS_IS_PLUGIN_LOADER_C(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_PLUGIN_LOADER_C))
#define PEAS_IS_PLUGIN_LOADER_C_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_PLUGIN_LOADER_C))
#define PEAS_PLUGIN_LOADER_C_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PEAS_TYPE_PLUGIN_LOADER_C, PeasPluginLoaderCClass))

typedef struct _PeasPluginLoaderC         PeasPluginLoaderC;
typedef struct _PeasPluginLoaderCClass    PeasPluginLoaderCClass;

struct _PeasPluginLoaderC {
  PeasPluginLoader parent;
};

struct _PeasPluginLoaderCClass {
  PeasPluginLoaderClass parent_class;
};

GType             peas_plugin_loader_c_get_type    (void) G_GNUC_CONST;
PeasPluginLoader *peas_plugin_loader_c_new         (void);

G_END_DECLS

#endif /* __PEAS_PLUGIN_LOADER_C_H__ */
