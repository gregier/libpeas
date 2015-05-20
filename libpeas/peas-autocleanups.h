/*
 * peas-autocleanups.h
 * This file is part of libpeas
 *
 * Copyright (C) 2015 - Garrett Regier
 *
 * libpeas is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libpeas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifndef __PEAS_AUTOCLEANUPS_H__
#define __PEAS_AUTOCLEANUPS_H__

#include "peas-activatable.h"
#include "peas-engine.h"
#include "peas-extension.h"
#include "peas-extension-base.h"
#include "peas-extension-set.h"
#include "peas-object-module.h"

G_BEGIN_DECLS

#ifndef __GI_SCANNER__
#if GLIB_CHECK_VERSION (2, 44, 0)

G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasActivatable, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasEngine, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasExtension, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasExtensionBase, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasExtensionSet, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasObjectModule, g_object_unref)

#endif /* GLIB_CHECK_VERSION (2, 44, 0) */
#endif /* __GI_SCANNER__ */

G_END_DECLS

#endif /* __PEAS_AUTOCLEANUPS_H__ */
