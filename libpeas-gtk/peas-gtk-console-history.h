/*
 * peas-gtk-console-history.h
 * This file is part of libpeas.
 *
 * Copyright (C) 2011 - Garrett Regier
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __PEAS_GTK_CONSOLE_HISTORY_H__
#define __PEAS_GTK_CONSOLE_HISTORY_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PEAS_GTK_TYPE_CONSOLE_HISTORY                (peas_gtk_console_history_get_type())

typedef struct _PeasGtkConsoleHistory PeasGtkConsoleHistory;

GType                  peas_gtk_console_history_get_type (void) G_GNUC_CONST;

PeasGtkConsoleHistory *peas_gtk_console_history_new      (void);

PeasGtkConsoleHistory *peas_gtk_console_history_ref      (PeasGtkConsoleHistory *history);
void                   peas_gtk_console_history_unref    (PeasGtkConsoleHistory *history);

void                   peas_gtk_console_history_add      (PeasGtkConsoleHistory *history,
                                                          const gchar           *text);
void                   peas_gtk_console_history_clear    (PeasGtkConsoleHistory *history);

void                   peas_gtk_console_history_next     (PeasGtkConsoleHistory *history);
void                   peas_gtk_console_history_previous (PeasGtkConsoleHistory *history);

const gchar           *peas_gtk_console_history_get      (PeasGtkConsoleHistory *history);

G_END_DECLS

#endif  /* __PEAS_GTK_CONSOLE_HISTORY_H__  */
