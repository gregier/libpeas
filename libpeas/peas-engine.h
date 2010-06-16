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

  void     (*load_plugin)                 (PeasEngine     *engine,
                                           PeasPluginInfo *info);

  void     (*unload_plugin)               (PeasEngine     *engine,
                                           PeasPluginInfo *info);
};

GType             peas_engine_get_type            (void) G_GNUC_CONST;
PeasEngine       *peas_engine_new                 (const gchar     *app_name,
                                                   const gchar     *base_module_dir,
                                                   const gchar    **search_paths);

/* plugin management */
void              peas_engine_disable_loader      (PeasEngine      *engine,
                                                   const gchar     *loader_id);
void              peas_engine_rescan_plugins      (PeasEngine      *engine);
const GList      *peas_engine_get_plugin_list     (PeasEngine      *engine);
gchar           **peas_engine_get_loaded_plugins  (PeasEngine      *engine);
void              peas_engine_set_loaded_plugins  (PeasEngine      *engine,
                                                   const gchar    **plugin_names);
PeasPluginInfo   *peas_engine_get_plugin_info     (PeasEngine      *engine,
                                                   const gchar     *plugin_name);

/* plugin loading and unloading */
gboolean          peas_engine_load_plugin         (PeasEngine      *engine,
                                                   PeasPluginInfo  *info);
gboolean          peas_engine_unload_plugin       (PeasEngine      *engine,
                                                   PeasPluginInfo  *info);
void              peas_engine_garbage_collect     (PeasEngine      *engine);

gboolean          peas_engine_provides_extension  (PeasEngine      *engine,
                                                   PeasPluginInfo  *info,
                                                   GType            extension_type);
PeasExtension    *peas_engine_get_extension       (PeasEngine      *engine,
                                                   PeasPluginInfo  *info,
                                                   GType            extension_type);


G_END_DECLS

#endif /* __PEAS_ENGINE_H__ */
