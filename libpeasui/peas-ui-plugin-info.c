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

/**
 * peas_ui_plugin_info_is_configurable:
 * @info: A #PeasPluginInfo
 *
 * Check if the plugin is configurable, given its #PeasPluginInfo.
 *
 * This function takes a #PeasPluginInfo as its argument and proxies the
 * is_configurable() method call to the #PeasPlugin, handling type checks and
 * casts for us. Calling this function on a #PeasPlugin which doesn't
 * implement #PeasUIConfigurable won't generate an error.
 *
 * Returns: %TRUE if the plugin is configurable.
 */
gboolean
peas_ui_plugin_info_is_configurable (PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  if (!peas_plugin_info_is_active (info))
    return FALSE;

  if (!PEAS_UI_IS_CONFIGURABLE (info->plugin))
    return FALSE;

  return peas_ui_configurable_is_configurable (PEAS_UI_CONFIGURABLE (info->plugin));
}

/**
 * peas_ui_plugin_info_create_configure_dialog:
 * @info: A #PeasPluginInfo
 *
 * Creates the configure dialog widget for the plugin, given its
 * #PeasPluginInfo.
 *
 * This function takes a #PeasPluginInfo as its argument and proxies the
 * is_configurable() method call to the #PeasPlugin, handling type checks and
 * casts for us. Calling this function on a #PeasPlugin which doesn't
 * implement #PeasUIConfigurable won't generate an error.
 *
 * Returns: a #GtkWindow instance.
 */
GtkWidget *
peas_ui_plugin_info_create_configure_dialog (PeasPluginInfo *info)
{
  PeasUIConfigurable *configurable;
  GtkWidget *dialog;

  /* Obligatory checks. */
  g_return_val_if_fail (info != NULL, NULL);

  if (!peas_plugin_info_is_active (info))
    return NULL;

  if (!PEAS_UI_IS_CONFIGURABLE (info->plugin))
    return NULL;

  configurable = PEAS_UI_CONFIGURABLE (info->plugin);
  dialog = peas_ui_configurable_create_configure_dialog (configurable);
  
  if (dialog != NULL && !GTK_IS_WINDOW (dialog)) {
    g_object_unref (dialog);
    dialog = NULL;
    g_return_val_if_reached (NULL);
  }

  return dialog;
}
