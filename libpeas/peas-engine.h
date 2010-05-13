/*
 * peas-engine.h
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
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

#ifndef __PEAS_ENGINE_H__
#define __PEAS_ENGINE_H__

#include <glib.h>
#include "peas-plugin-info.h"
#include "peas-plugin.h"
#include "peas-extension.h"

G_BEGIN_DECLS

#define PEAS_TYPE_ENGINE              (peas_engine_get_type ())
#define PEAS_ENGINE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_TYPE_ENGINE, PeasEngine))
#define PEAS_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_TYPE_ENGINE, PeasEngineClass))
#define PEAS_IS_ENGINE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_TYPE_ENGINE))
#define PEAS_IS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_ENGINE))
#define PEAS_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_TYPE_ENGINE, PeasEngineClass))

/**
 * PeasEngine:
 * @parent: the parent object.
 *
 * Engine at the heart of the Peas plugin system.
 */
typedef struct _PeasEngine        PeasEngine;
typedef struct _PeasEngineClass   PeasEngineClass;
typedef struct _PeasEnginePrivate PeasEnginePrivate;

struct _PeasEngine {
  GObject parent;

  /*< private > */
  PeasEnginePrivate *priv;
};

struct _PeasEngineClass {
  GObjectClass parent_class;

  void     (*activate_plugin)             (PeasEngine     *engine,
                                           PeasPluginInfo *info);

  void     (*deactivate_plugin)           (PeasEngine     *engine,
                                           PeasPluginInfo *info);

  void     (*activate_plugin_on_object)   (PeasEngine     *engine,
                                           PeasPluginInfo *info,
                                           GObject        *object);

  void     (*deactivate_plugin_on_object) (PeasEngine     *engine,
                                           PeasPluginInfo *info,
                                           GObject        *object);
};

GType             peas_engine_get_type            (void) G_GNUC_CONST;
PeasEngine       *peas_engine_new                 (const gchar     *app_name,
                                                   const gchar     *base_module_dir,
                                                   const gchar    **search_paths);

/* plugin management */
void              peas_engine_rescan_plugins      (PeasEngine      *engine);
const GList      *peas_engine_get_plugin_list     (PeasEngine      *engine);
gchar           **peas_engine_get_active_plugins  (PeasEngine      *engine);
void              peas_engine_set_active_plugins  (PeasEngine      *engine,
                                                   const gchar    **plugin_names);
PeasPluginInfo   *peas_engine_get_plugin_info     (PeasEngine      *engine,
                                                   const gchar     *plugin_name);

/* plugin load and unloading (overall, for all windows) */
gboolean          peas_engine_activate_plugin     (PeasEngine      *engine,
                                                   PeasPluginInfo  *info);
gboolean          peas_engine_deactivate_plugin   (PeasEngine      *engine,
                                                   PeasPluginInfo  *info);
void              peas_engine_garbage_collect     (PeasEngine      *engine);

PeasExtension    *peas_engine_get_extension       (PeasEngine      *engine,
                                                   PeasPluginInfo  *info,
                                                   GType            ext_type);

/* plugin activation/deactivation per target_object */
void              peas_engine_update_plugins_ui   (PeasEngine      *engine,
                                                   GObject         *object);

/* object management */
void              peas_engine_add_object          (PeasEngine      *engine,
                                                   GObject         *object);
void              peas_engine_remove_object       (PeasEngine      *engine,
                                                   GObject         *object);

G_END_DECLS

#endif /* __PEAS_ENGINE_H__ */
