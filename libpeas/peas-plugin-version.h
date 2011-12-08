/*
 * peas-plugin-version.h
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

#ifndef __PEAS_PLUGIN_VERSION_H__
#define __PEAS_PLUGIN_VERSION_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define PEAS_TYPE_PLUGIN_VERSION   (peas_plugin_version_get_type ())

/**
 * PeasPluginVersion:
 *
 * The #PeasPluginVersion structure contains only private data
 * and should only be accessed using the provided API.
 */
typedef struct _PeasPluginVersion PeasPluginVersion;

/**
 * PeasPluginVersionOperation:
 * @PEAS_PLUGIN_VERSION_OPERATION_EQ:  Equal.
 * @PEAS_PLUGIN_VERSION_OPERATION_NE:  Not equal.
 * @PEAS_PLUGIN_VERSION_OPERATION_GT:  Greater than.
 * @PEAS_PLUGIN_VERSION_OPERATION_LT:  Less than.
 * @PEAS_PLUGIN_VERSION_OPERATION_GTE: Greater than or equal.
 * @PEAS_PLUGIN_VERSION_OPERATION_LTE: Less than or equal.
 *
 * Describes the operation to perform when checking a version.
 */
typedef enum {
  PEAS_PLUGIN_VERSION_OPERATION_EQ  = 1 << 0,
  PEAS_PLUGIN_VERSION_OPERATION_NE  = 1 << 1,
  PEAS_PLUGIN_VERSION_OPERATION_GT  = 1 << 2,
  PEAS_PLUGIN_VERSION_OPERATION_LT  = 1 << 3,
  PEAS_PLUGIN_VERSION_OPERATION_GTE = (PEAS_PLUGIN_VERSION_OPERATION_EQ |
                                       PEAS_PLUGIN_VERSION_OPERATION_GT),
  PEAS_PLUGIN_VERSION_OPERATION_LTE = (PEAS_PLUGIN_VERSION_OPERATION_EQ |
                                       PEAS_PLUGIN_VERSION_OPERATION_LT)
} PeasPluginVersionOperation;


GType     peas_plugin_version_get_type (void) G_GNUC_CONST;
PeasPluginVersion *
          peas_plugin_version_new      (const gchar                *version_str);

PeasPluginVersion *
          peas_plugin_version_ref      (PeasPluginVersion          *version);
void      peas_plugin_version_unref    (PeasPluginVersion          *version);

gboolean  peas_plugin_version_check    (const PeasPluginVersion    *a,
                                        const PeasPluginVersion    *b,
                                        PeasPluginVersionOperation  op);

gchar    *peas_plugin_version_to_string (const PeasPluginVersion *version);

G_END_DECLS

#endif /* __PEAS_PLUGIN_VERSION_H__ */
