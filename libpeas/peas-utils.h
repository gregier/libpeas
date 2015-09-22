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

#define PEAS_UTILS_C_LOADER_ID  0
#define PEAS_UTILS_N_LOADERS    4

gboolean  peas_utils_valist_to_parameter_list (GType         iface_type,
                                               const gchar  *first_property,
                                               va_list       var_args,
                                               GParameter  **params,
                                               guint        *n_params);

gint     peas_utils_get_loader_id             (const gchar  *loader) G_GNUC_CONST;
const gchar *
         peas_utils_get_loader_from_id        (gint          loader_id) G_GNUC_CONST;
const gchar *
         peas_utils_get_loader_module_from_id (gint          loader_id) G_GNUC_CONST;
const gint *
         peas_utils_get_conflicting_loaders_from_id
                                              (gint          loader_id) G_GNUC_CONST;

#endif /* __PEAS_UTILS_H__ */
