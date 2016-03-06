/*
 * peas-gtk-disable-plugins-dialog.h
 * This file is part of libpeas
 *
 * Copyright (C) 2011 Garrett Regier
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

#ifndef __PEAS_GTK_DISABLE_PLUGINS_DIALOG_H__
#define __PEAS_GTK_DISABLE_PLUGINS_DIALOG_H__

#include <gtk/gtk.h>
#include <libpeas/peas-plugin-info.h>

G_BEGIN_DECLS

GType      peas_gtk_disable_plugins_dialog_get_type (void) G_GNUC_CONST;

GtkWidget *peas_gtk_disable_plugins_dialog_new      (GtkWindow      *parent,
                                                     PeasPluginInfo *info,
                                                     GList          *dep_plugins);

G_END_DECLS

#endif /* __PEAS_GTK_DISABLE_PLUGINS_DIALOG_H__  */
