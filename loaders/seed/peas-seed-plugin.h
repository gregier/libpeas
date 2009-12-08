/*
 * peas-seed-plugin.h
 * This file is part of libpeas
 *
 * Copyright (C) 2009 - Steve Frécinaux
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

#ifndef __PEAS_SEED_PLUGIN_H__
#define __PEAS_SEED_PLUGIN_H__

#include <libpeas/peas-plugin.h>
#include <seed.h>

G_BEGIN_DECLS

#define PEAS_TYPE_SEED_PLUGIN            (peas_seed_plugin_get_type ())
#define PEAS_SEED_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_SEED_PLUGIN, PeasSeedPlugin))
#define PEAS_SEED_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PEAS_TYPE_SEED_PLUGIN, PeasSeedPluginClass))
#define PEAS_IS_SEED_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_SEED_PLUGIN))
#define PEAS_IS_SEED_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_SEED_PLUGIN))
#define PEAS_SEED_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PEAS_TYPE_SEED_PLUGIN, PeasSeedPluginClass))

typedef struct _PeasSeedPlugin        PeasSeedPlugin;
typedef struct _PeasSeedPluginClass   PeasSeedPluginClass;

struct _PeasSeedPlugin {
  PeasPlugin parent;

  SeedContext js_context;
  SeedObject js_plugin;
};

struct _PeasSeedPluginClass {
  PeasPluginClass parent_class;
};

GType            peas_seed_plugin_get_type          (void) G_GNUC_CONST;
void             peas_seed_plugin_register_type_ext (GTypeModule    *type_module);

PeasPlugin      *peas_seed_plugin_new               (PeasPluginInfo *info,
                                                     SeedContext     js_context,
                                                     SeedObject      js_plugin);

G_END_DECLS

#endif /* __PEAS_SEED_PLUGIN_H__ */

