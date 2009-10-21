/*
 * gpe-dirs.c
 * This file is part of libgpe
 *
 * Copyright (C) 2008 Ignacio Casal Quinteiro
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

#include "gpe-dirs.h"

gchar *
gpe_dirs_get_data_dir (void)
{
	return g_build_filename (DATADIR, "libgpe-2.0", NULL);
}

gchar *
gpe_dirs_get_lib_dir (void)
{
	return g_build_filename (LIBDIR, "libgpe-2.0",  NULL);
}

gchar *
gpe_dirs_get_user_plugins_dir (void)
{
	return g_build_filename (g_get_user_config_dir (), "libgpe", "plugins", NULL);
}

gchar *
gpe_dirs_get_plugins_dir (void)
{
	gchar *lib_dir;
	gchar *plugin_dir;
	
	lib_dir = gpe_dirs_get_lib_dir ();
	plugin_dir = g_build_filename (lib_dir, "plugins",  NULL);

	g_free (lib_dir);
	
	return plugin_dir;
}

gchar *
gpe_dirs_get_plugin_loaders_dir (void)
{
	gchar *lib_dir;
	gchar *loader_dir;
	
	lib_dir = gpe_dirs_get_lib_dir ();
	loader_dir = g_build_filename (lib_dir, "loaders", NULL);

	g_free (lib_dir);
	
	return loader_dir;
}
