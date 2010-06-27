/*
 * peas-ui-plugin-info.c
 * This file is part of libpeasui
 *
 * Copyright (C) 2009 Steve Steve Frécinaux
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libpeas/peas-plugin-info-priv.h>

#include "peas-ui-plugin-info.h"
#include "peas-ui-configurable.h"

/**
 * SECTION:peas-ui-plugin-info
 * @short_description: Some UI-related methods for #PeasPluginInfo.
 * @see_also: #PeasPluginInfo
 *
 * This summaries a few extra methods for #PeasPluginInfo, to ease the
 * management of the plugins in a GUI context. Those have the benefit of being
 * callable on any #PeasPluginInfo, even if the related plugin isn't active or
 * doesn't implement the required interfaces.
 **/

/**
 * peas_ui_plugin_info_get_icon_name:
 * @info: A #PeasPluginInfo.
 *
 * Gets the icon name of the plugin.
 *
 * Returns: the plugin's icon name.
 */
const gchar *
peas_ui_plugin_info_get_icon_name (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  /* use the libpeas-plugin icon as a default if the plugin does not
     have its own */
  if (info->icon_name != NULL &&
      gtk_icon_theme_has_icon (gtk_icon_theme_get_default (),
                               info->icon_name))
    return info->icon_name;
  else
    return "libpeas-plugin";
}
