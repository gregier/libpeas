/*
 * peas-ui-plugin-info.h
 * This file is part of libpeasui
 *
 * Copyright (C) 2009 Steve Frécinaux
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

#ifndef __PEAS_UI_PLUGIN_INFO_H__
#define __PEAS_UI_PLUGIN_INFO_H__

#include <gtk/gtk.h>
#include <libpeas/peas-plugin-info.h>

G_BEGIN_DECLS

gboolean    peas_ui_plugin_info_is_configurable          (PeasPluginInfo *info);
GtkWidget  *peas_ui_plugin_info_create_configure_dialog  (PeasPluginInfo *info);

G_END_DECLS

#endif /* __PEAS_UI_PLUGIN_INFO_H__ */
