/*
 * gpe-plugins-info.h
 * This file is part of libgpe
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2007 - Steve Frécinaux
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

#ifndef __GPE_PLUGIN_INFO_H__
#define __GPE_PLUGIN_INFO_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GPE_TYPE_PLUGIN_INFO			(gpe_plugin_info_get_type ())
#define GPE_PLUGIN_INFO(obj)			((GPEPluginInfo *) (obj))

typedef struct _GPEPluginInfo			GPEPluginInfo;

GType		 gpe_plugin_info_get_type		(void) G_GNUC_CONST;

gboolean 	 gpe_plugin_info_is_active		(GPEPluginInfo *info);
gboolean 	 gpe_plugin_info_is_available		(GPEPluginInfo *info);
gboolean	 gpe_plugin_info_is_configurable	(GPEPluginInfo *info);

const gchar	*gpe_plugin_info_get_module_name	(GPEPluginInfo *info);
const gchar	*gpe_plugin_info_get_module_dir		(GPEPluginInfo *info);
const gchar	*gpe_plugin_info_get_data_dir		(GPEPluginInfo *info);

const gchar	*gpe_plugin_info_get_name		(GPEPluginInfo *info);
const gchar	*gpe_plugin_info_get_description	(GPEPluginInfo *info);
const gchar	*gpe_plugin_info_get_icon_name		(GPEPluginInfo *info);
const gchar    **gpe_plugin_info_get_authors		(GPEPluginInfo *info);
const gchar	*gpe_plugin_info_get_website		(GPEPluginInfo *info);
const gchar	*gpe_plugin_info_get_copyright		(GPEPluginInfo *info);
const gchar	*gpe_plugin_info_get_version		(GPEPluginInfo *info);

G_END_DECLS

#endif /* __GPE_PLUGIN_INFO_H__ */

