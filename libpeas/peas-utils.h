/*
 * peas-utils.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
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

#ifndef __PEAS_UTILS_H__
#define __PEAS_UTILS_H__

#include <glib-object.h>

typedef enum {
  PEAS_UTILS_INVALID_LOADER_ID = -1,
  PEAS_UTILS_C_LOADER_ID,
  PEAS_UTILS_LUA51_LOADER_ID,
  PEAS_UTILS_PYTHON_LOADER_ID,
  PEAS_UTILS_PYTHON3_LOADER_ID,
  PEAS_UTILS_N_LOADERS
} PeasUtilsLoaderID;

GArray      *peas_utils_valist_to_parameter_list (GType              iface_type,
                                                  const gchar       *first_property,
                                                  va_list            var_args);

PeasUtilsLoaderID
             peas_utils_get_loader_id            (const gchar       *loader) G_GNUC_CONST;
const gchar *peas_utils_get_loader_name          (PeasUtilsLoaderID  loader_id) G_GNUC_CONST;
const gchar *peas_utils_get_loader_module_name   (PeasUtilsLoaderID  loader_id) G_GNUC_CONST;
const PeasUtilsLoaderID *
             peas_utils_get_conflicting_loaders  (PeasUtilsLoaderID  loader_id) G_GNUC_CONST;

#endif /* __PEAS_UTILS_H__ */
