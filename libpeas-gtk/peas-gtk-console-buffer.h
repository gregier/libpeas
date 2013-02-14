/*
 * peas-gtk-console-buffer.h
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

#ifndef __PEAS_GTK_CONSOLE_BUFFER_H__
#define __PEAS_GTK_CONSOLE_BUFFER_H__

#include <gtk/gtk.h>

#include <libpeas/peas-interpreter.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_GTK_TYPE_CONSOLE_BUFFER            (peas_gtk_console_buffer_get_type())
#define PEAS_GTK_CONSOLE_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_GTK_TYPE_CONSOLE_BUFFER, PeasGtkConsoleBuffer))
#define PEAS_GTK_CONSOLE_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_GTK_TYPE_CONSOLE_BUFFER, PeasGtkConsoleBufferClass))
#define PEAS_GTK_IS_CONSOLE_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_GTK_TYPE_CONSOLE_BUFFER))
#define PEAS_GTK_IS_CONSOLE_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_GTK_TYPE_CONSOLE_BUFFER))
#define PEAS_GTK_CONSOLE_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_GTK_TYPE_CONSOLE_BUFFER, PeasGtkConsoleBufferClass))

typedef struct _PeasGtkConsoleBuffer        PeasGtkConsoleBuffer;
typedef struct _PeasGtkConsoleBufferClass   PeasGtkConsoleBufferClass;

struct _PeasGtkConsoleBuffer {
  GtkTextBuffer parent;
};

struct _PeasGtkConsoleBufferClass {
  GtkTextBufferClass parent_instance;
};

GType                 peas_gtk_console_buffer_get_type          (void) G_GNUC_CONST;

PeasGtkConsoleBuffer *peas_gtk_console_buffer_new               (PeasInterpreter       *interpreter);

PeasInterpreter      *peas_gtk_console_buffer_get_interpreter   (PeasGtkConsoleBuffer  *buffer);

void                  peas_gtk_console_buffer_execute           (PeasGtkConsoleBuffer  *buffer);
void                  peas_gtk_console_buffer_write_prompt      (PeasGtkConsoleBuffer  *buffer);
void                  peas_gtk_console_buffer_write_completions (PeasGtkConsoleBuffer  *buffer);

gchar                *peas_gtk_console_buffer_get_statement     (PeasGtkConsoleBuffer  *buffer) G_GNUC_WARN_UNUSED_RESULT;
void                  peas_gtk_console_buffer_set_statement     (PeasGtkConsoleBuffer  *buffer,
                                                                 const gchar           *statement);


GtkTextMark          *peas_gtk_console_buffer_get_input_mark    (PeasGtkConsoleBuffer  *buffer);
void                  peas_gtk_console_buffer_get_input_iter    (PeasGtkConsoleBuffer  *buffer,
                                                                 GtkTextIter           *iter);

void                  peas_gtk_console_buffer_move_cursor       (PeasGtkConsoleBuffer  *buffer,
                                                                 GtkTextIter           *iter,
                                                                 gboolean               extend_selection);

G_END_DECLS

#endif  /* __PEAS_GTK_CONSOLE_BUFFER_H__  */
