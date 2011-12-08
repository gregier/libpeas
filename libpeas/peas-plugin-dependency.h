/*
 * peas-plugin-dependency.h
 * This file is part of libpeas
 *
 * Copyright (C) 2011 - Garrett Regier
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

#ifndef __PEAS_PLUGIN_DEPENDENCY_H__
#define __PEAS_PLUGIN_DEPENDENCY_H__

#include <glib-object.h>

#include "peas-plugin-version.h"

G_BEGIN_DECLS

#define PEAS_TYPE_PLUGIN_DEPENDENCY   (peas_plugin_dependency_get_type ())

/**
 * PeasPluginDependency:
 *
 * The #PeasPluginDependency structure contains only private data
 * and should only be accessed using the provided API.
 */
typedef struct _PeasPluginDependency PeasPluginDependency;


GType        peas_plugin_dependency_get_type      (void) G_GNUC_CONST;
PeasPluginDependency *
             peas_plugin_dependency_new           (const gchar                *dep_str);

PeasPluginDependency *
             peas_plugin_dependency_ref           (PeasPluginDependency       *dep);
void         peas_plugin_dependency_unref         (PeasPluginDependency       *dep);

gboolean     peas_plugin_dependency_check         (const PeasPluginDependency *dep,
                                                   const gchar                *version_str);
gboolean     peas_plugin_dependency_check_version (const PeasPluginDependency *dep,
                                                   const PeasPluginVersion    *version);

const gchar *peas_plugin_dependency_get_name      (const PeasPluginDependency *dep);
gchar       *peas_plugin_dependency_to_string     (const PeasPluginDependency *dep);

G_END_DECLS

#endif /* __PEAS_PLUGIN_DEPENDENCY_H__ */
