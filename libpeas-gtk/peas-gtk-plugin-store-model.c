/*
 * peas-gtk-plugin-store-model.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 Garrett Regier
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

#include <libpeas/peas-plugin-info.h>

#include "peas-gtk-plugin-store-model.h"

static const GType ColumnTypes[] = {
  G_TYPE_BOOLEAN, /* Installed */
  G_TYPE_BOOLEAN, /* Not In Use */
  G_TYPE_BOOLEAN, /* Available */
  G_TYPE_BOOLEAN, /* Active (ProgressBar is shown) */
  G_TYPE_INT,     /* Pulse */
  G_TYPE_INT,     /* Progress */
  G_TYPE_STRING,  /* Progress Message */
  G_TYPE_OBJECT,  /* Pixbuf Icon */
  G_TYPE_STRING,  /* Stock Icon */
  G_TYPE_STRING,  /* Info */
  /* To avoid having to unref it all the time */
  G_TYPE_POINTER  /* PeasGtkInstallablePluginInfo */
};

G_STATIC_ASSERT (G_N_ELEMENTS (ColumnTypes) == PEAS_GTK_PLUGIN_STORE_MODEL_N_COLUMNS);

struct _PeasGtkPluginStoreModelPrivate {
  PeasEngine *engine;
  /*PeasGtkPluginStoreBackend *backend;*/
};

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE (PeasGtkPluginStoreModel, peas_gtk_plugin_store_model, GTK_TYPE_LIST_STORE)

static void
update_plugin (PeasGtkPluginStoreModel            *model,
               GtkTreeIter                        *iter,
               const PeasGtkInstallablePluginInfo *info,
               gboolean                            reloading)
{
  gboolean installed;
  gboolean in_use;
  gboolean available;
  gchar *markup;
  const gchar *icon_name;
  GdkPixbuf *icon_pixbuf = NULL;

  installed = peas_gtk_installable_plugin_info_is_installed (info);
  in_use = peas_gtk_installable_plugin_info_is_in_use (info);
  available = peas_gtk_installable_plugin_info_is_available (info, NULL);

  gtk_list_store_set (GTK_LIST_STORE (model), iter,
    PEAS_GTK_PLUGIN_STORE_MODEL_INSTALLED_COLUMN,   installed,
    PEAS_GTK_PLUGIN_STORE_MODEL_NOT_IN_USE_COLUMN,  !in_use,
    PEAS_GTK_PLUGIN_STORE_MODEL_AVAILABLE_COLUMN,   available,
    PEAS_GTK_PLUGIN_STORE_MODEL_ACTIVE_COLUMN,      in_use,
    -1);

  if (reloading)
    return;

  if (peas_gtk_installable_plugin_info_get_description (info) == NULL)
    {
      markup = g_markup_printf_escaped ("<b>%s</b>",
                                        peas_gtk_installable_plugin_info_get_name (info));
    }
  else
    {
      markup = g_markup_printf_escaped ("<b>%s</b>\n%s",
                                        peas_gtk_installable_plugin_info_get_name (info),
                                        peas_gtk_installable_plugin_info_get_description (info));
    }

  if (!peas_gtk_installable_plugin_info_is_available (info, NULL))
    {
      icon_name = GTK_STOCK_DIALOG_ERROR;
      icon_pixbuf = NULL;
    }
  else
    {
      icon_name = peas_gtk_installable_plugin_info_get_icon_name (info);

      /* Prevent warning for the common case that icon_name
       * does not exist but warn when it is a directory
       */
      if (g_file_test (icon_name, G_FILE_TEST_EXISTS))
        {
          GError *error = NULL;
          gint width, height;

          /* Attempt to load the icon scaled to the correct size */
          if (!gtk_icon_size_lookup (GTK_ICON_SIZE_SMALL_TOOLBAR,
                                     &width, &height))
            {
              icon_pixbuf = gdk_pixbuf_new_from_file (icon_name, &error);
            }
          else
            {
              icon_pixbuf = gdk_pixbuf_new_from_file_at_size (icon_name,
                                                              width, height,
                                                              &error);
            }

          if (error == NULL)
            {
              icon_name = NULL;
            }
          else
            {
              g_warning ("Error while loading icon: %s", error->message);
              g_error_free (error);
            }
        }

      if (icon_pixbuf == NULL &&
          !gtk_icon_theme_has_icon (gtk_icon_theme_get_default (), icon_name))
        icon_name = "libpeas-plugin";
    }

  gtk_list_store_set (GTK_LIST_STORE (model), iter,
    PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN,            -1,
    PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_COLUMN,         0,
    PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_MESSAGE_COLUMN, "",
    PEAS_GTK_PLUGIN_STORE_MODEL_ICON_PIXBUF_COLUMN,      icon_pixbuf,
    PEAS_GTK_PLUGIN_STORE_MODEL_ICON_NAME_COLUMN,        icon_name,
    PEAS_GTK_PLUGIN_STORE_MODEL_INFO_COLUMN,             markup,
    PEAS_GTK_PLUGIN_STORE_MODEL_PLUGIN_COLUMN,           info,
    -1);

  if (icon_pixbuf != NULL)
    g_object_unref (icon_pixbuf);

  g_free (markup);
}

static gint
model_name_sort_func (PeasGtkPluginStoreModel *model,
                      GtkTreeIter             *iter1,
                      GtkTreeIter             *iter2,
                      gpointer                 user_data)
{
  PeasGtkInstallablePluginInfo *info1;
  PeasGtkInstallablePluginInfo *info2;

  info1 = peas_gtk_plugin_store_model_get_plugin (model, iter1);
  info2 = peas_gtk_plugin_store_model_get_plugin (model, iter2);

  if (info1 == NULL || info2 == NULL)
    return 0;

  return g_utf8_collate (peas_gtk_installable_plugin_info_get_name (info1),
                         peas_gtk_installable_plugin_info_get_name (info2));
}

static void
peas_gtk_plugin_store_model_init (PeasGtkPluginStoreModel *model)
{
  model->priv = G_TYPE_INSTANCE_GET_PRIVATE (model,
                                             PEAS_GTK_TYPE_PLUGIN_STORE_MODEL,
                                             PeasGtkPluginStoreModelPrivate);

  gtk_list_store_set_column_types (GTK_LIST_STORE (model),
                                    PEAS_GTK_PLUGIN_STORE_MODEL_N_COLUMNS,
                                    (GType *) ColumnTypes);

  /* Sort on the plugin names */
  gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (model),
                                           (GtkTreeIterCompareFunc) model_name_sort_func,
                                           NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
                                        GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                        GTK_SORT_ASCENDING);
}

static void
peas_gtk_plugin_store_model_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  PeasGtkPluginStoreModel *model = PEAS_GTK_PLUGIN_STORE_MODEL (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      model->priv->engine = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_plugin_store_model_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  PeasGtkPluginStoreModel *model = PEAS_GTK_PLUGIN_STORE_MODEL (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, model->priv->engine);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_plugin_store_model_constructed (GObject *object)
{
  PeasGtkPluginStoreModel *model = PEAS_GTK_PLUGIN_STORE_MODEL (object);

  if (model->priv->engine == NULL)
    model->priv->engine = peas_engine_get_default ();

  g_object_ref (model->priv->engine);

  G_OBJECT_CLASS (peas_gtk_plugin_store_model_parent_class)->constructed (object);
}

static void
peas_gtk_plugin_store_model_dispose (GObject *object)
{
  PeasGtkPluginStoreModel *model = PEAS_GTK_PLUGIN_STORE_MODEL (object);

  g_clear_object (&model->priv->engine);

  G_OBJECT_CLASS (peas_gtk_plugin_store_model_parent_class)->dispose (object);
}

static void
peas_gtk_plugin_store_model_class_init (PeasGtkPluginStoreModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_gtk_plugin_store_model_set_property;
  object_class->get_property = peas_gtk_plugin_store_model_get_property;
  object_class->constructed = peas_gtk_plugin_store_model_constructed;
  object_class->dispose = peas_gtk_plugin_store_model_dispose;

  /**
   * PeasGtkPLuginStoreModel:engine:
   *
   * The #PeasEngine this model is attached to.
   */
  properties[PROP_ENGINE] =
    g_param_spec_object ("engine",
                         "engine",
                         "The PeasEngine this model is attached to",
                         PEAS_TYPE_ENGINE,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
  g_type_class_add_private (object_class, sizeof (PeasGtkPluginStoreModelPrivate));
}

/**
 * peas_gtk_plugin_store_model_new:
 * @engine: (allow-none): A #PeasEngine, or %NULL.
 *
 * Creates a new plugin store model for the given #PeasEngine.
 *
 * If @engine is %NULL, then the default engine will be used.
 *
 * Returns: the new #PeasGtkPluginStoreModel.
 */
PeasGtkPluginStoreModel  *
peas_gtk_plugin_store_model_new (PeasEngine *engine)
{
  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);

  return PEAS_GTK_PLUGIN_STORE_MODEL (g_object_new (PEAS_GTK_TYPE_PLUGIN_STORE_MODEL,
                                                    "engine", engine,
                                                    NULL));
}

/**
 * peas_gtk_plugin_store_model_reload:
 * @model: A #PeasGtkPluginStoreModel.
 * @plugins: (allow-none): A #GPtrArray of #PeasGtkInstallablePluginInfos.
 *
 * Reloads the store of plugins.
 */
void
peas_gtk_plugin_store_model_reload (PeasGtkPluginStoreModel *model,
                                    GPtrArray               *plugins)
{
  GtkListStore *store;
  guint i;
  GtkTreeIter iter;

  g_return_if_fail (PEAS_GTK_IS_PLUGIN_STORE_MODEL (model));

  store = GTK_LIST_STORE (model);

  gtk_list_store_clear (store);

  for (i = 0; plugins != NULL && i < plugins->len; ++i)
    {
      PeasGtkInstallablePluginInfo *info = g_ptr_array_index (plugins, i);

      gtk_list_store_append (store, &iter);
      update_plugin (model, &iter, info, FALSE);
    }
}

/**
 * peas_gtk_plugin_store_model_reload_plugin:
 * @model: A #PeasGtkPluginStoreModel.
 * @info: The #PeasGtkInstallablePluginInfo to reload.
 *
 * Reloads the store's data for @info.
 */
void
peas_gtk_plugin_store_model_reload_plugin (PeasGtkPluginStoreModel            *model,
                                           const PeasGtkInstallablePluginInfo *info)
{
  GtkTreeIter iter;

  g_return_if_fail (peas_gtk_plugin_store_model_get_iter_from_plugin (model, &iter, info));

  update_plugin (model, &iter, info, TRUE);
}

/**
 * peas_gtk_plugin_store_model_get_plugin:
 * @model: A #PeasGtkPluginStoreModel.
 * @iter: A #GtkTreeIter.
 *
 * Returns the plugin at @iter.
 *
 * Returns: the plugin at @iter.
 */
PeasGtkInstallablePluginInfo *
peas_gtk_plugin_store_model_get_plugin (PeasGtkPluginStoreModel *model,
                                        GtkTreeIter             *iter)
{
  GValue value = { 0 };
  PeasGtkInstallablePluginInfo *info;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_STORE_MODEL (model), NULL);
  g_return_val_if_fail (iter != NULL, NULL);

  gtk_tree_model_get_value (GTK_TREE_MODEL (model), iter,
                            PEAS_GTK_PLUGIN_STORE_MODEL_PLUGIN_COLUMN, &value);

  g_return_val_if_fail (G_VALUE_HOLDS_POINTER (&value), NULL);
  info = g_value_get_pointer (&value);

  g_value_unset (&value);

  /* We register it as a pointer instead
   * of a boxed so no need to unref it
   */
  return info;
}

/**
 * peas_gtk_plugin_store_model_get_iter_from_plugin:
 * @model: A #PeasGtkPluginStoreModel.
 * @iter: A #GtkTreeIter.
 * @info: A #PeasGtkInstallablePluginInfo.
 *
 * Sets @iter to the @info.
 *
 * Returns: if @iter was set.
 */
gboolean
peas_gtk_plugin_store_model_get_iter_from_plugin (PeasGtkPluginStoreModel            *model,
                                                  GtkTreeIter                        *iter,
                                                  const PeasGtkInstallablePluginInfo *info)
{
  GtkTreeModel *tree_model;
  gboolean found = FALSE;

  g_return_val_if_fail (PEAS_GTK_IS_PLUGIN_STORE_MODEL (model), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (info != NULL, FALSE);

  tree_model = GTK_TREE_MODEL (model);

  if (gtk_tree_model_get_iter_first (tree_model, iter))
    {
      PeasGtkInstallablePluginInfo *current_info;

      do
        {
          current_info = peas_gtk_plugin_store_model_get_plugin (model, iter);

          found = (info == current_info);
        }
      while (!found && gtk_tree_model_iter_next (tree_model, iter));
    }

  return found;
}
