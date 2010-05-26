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
