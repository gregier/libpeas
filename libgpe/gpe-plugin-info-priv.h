/*
 * gpe-plugin-info-priv.h
 * This file is part of libgpe
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2007 - Steve Frécinaux
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GPE_PLUGIN_INFO_PRIV_H__
#define __GPE_PLUGIN_INFO_PRIV_H__

#include "gpe-plugin-info.h"
#include "gpe-plugin.h"
#include "gpe-path-info.h"


struct _GPEPluginInfo
{
	gint               refcount;

	GPEPlugin         *plugin;
	gchar             *file;
	gchar             *module_dir;
	gchar             *data_dir;

	gchar             *module_name;
	gchar		  *loader;
	gchar            **dependencies;

	gchar             *name;
	gchar             *desc;
	gchar             *icon_name;
	gchar            **authors;
	gchar             *copyright;
	gchar             *website;
	gchar             *version;

	/* A plugin is unavailable if it is not possible to activate it
	   due to an error loading the plugin module (e.g. for Python plugins
	   when the interpreter has not been correctly initializated) */
	gint               available : 1;
};

GPEPluginInfo	*_gpe_plugin_info_new		(const gchar *file, const GPEPathInfo *pathinfo);
void		 _gpe_plugin_info_ref		(GPEPluginInfo *info);
void		 _gpe_plugin_info_unref		(GPEPluginInfo *info);

#endif /* __GPE_PLUGIN_INFO_PRIV_H__ */
