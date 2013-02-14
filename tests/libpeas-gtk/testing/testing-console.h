/*
 * testing-console.h
 * This file is part of libpeas
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

#ifndef __TESTING_CONSOLE_H__
#define __TESTING_CONSOLE_H__

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

G_BEGIN_DECLS

GtkTextView  *
         testing_console_get_view            (PeasGtkConsole *console);
GtkTextBuffer *
         testing_console_get_buffer          (PeasGtkConsole *console);

/* Movement Keybindings */
void      testing_console_move_up            (PeasGtkConsole *console);
void      testing_console_move_down          (PeasGtkConsole *console);
void      testing_console_move_left          (PeasGtkConsole *console);
void      testing_console_move_right         (PeasGtkConsole *console);
void      testing_console_move_home          (PeasGtkConsole *console);
void      testing_console_move_end           (PeasGtkConsole *console);

/* Selection Keybindings */
void      testing_console_select_all         (PeasGtkConsole *console);
void      testing_console_select_left        (PeasGtkConsole *console,
                                              guint           n);
void      testing_console_select_right       (PeasGtkConsole *console,
                                              guint           n);
void      testing_console_select_up          (PeasGtkConsole *console,
                                              guint           n);
void      testing_console_select_down        (PeasGtkConsole *console,
                                              guint           n);

/* Other Keybindings */
void      testing_console_backspace          (PeasGtkConsole *console);
void      testing_console_complete           (PeasGtkConsole *console);
void      testing_console_reset              (PeasGtkConsole *console);

/* Helpers */
void      testing_console_write              (PeasGtkConsole *console,
                                              const gchar    *text);
void      testing_console_execute            (PeasGtkConsole *console,
                                              const gchar    *code);
void      testing_console_set_input_pos      (PeasGtkConsole *console,
                                              guint           pos);
gboolean  testing_console_get_input_iter     (PeasGtkConsole *console,
                                              GtkTextIter    *iter);
gchar    *testing_console_get_statement_repr (PeasGtkConsole *console);


/* Implemented as macros so we get correct
 * line numbers when an assert fails.
 */
#define assert_console_text(console, expected) \
  G_STMT_START \
    { \
      GtkTextBuffer *buffer; \
      GtkTextIter start, end; \
      gchar *text; \
\
      buffer = testing_console_get_buffer (console); \
      gtk_text_buffer_get_bounds (buffer, &start, &end); \
\
      text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE); \
\
      g_assert_cmpstr (text, ==, expected); \
\
      g_free (text); \
    } \
  G_STMT_END

#define assert_console_cursor_pos(console, expected) \
  G_STMT_START \
    { \
      gchar *statement_repr; \
\
      statement_repr = testing_console_get_statement_repr (console); \
\
      g_assert_cmpstr (statement_repr, ==, expected); \
\
      g_free (statement_repr); \
    } \
  G_STMT_END

#define assert_console_selection(console, expected) \
  G_STMT_START \
    { \
      GtkTextBuffer *buffer; \
      GtkTextIter start, end; \
      gboolean has_selection; \
\
      buffer = testing_console_get_buffer (console); \
      has_selection = gtk_text_buffer_get_selection_bounds (buffer, \
                                                            &start, &end); \
\
      if (expected == NULL) \
        { \
          g_assert (!has_selection); \
        } \
      else \
        { \
          gchar *text; \
\
          g_assert (has_selection); \
\
          text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE); \
\
          g_assert_cmpstr (text, ==, expected); \
\
          g_free (text); \
      } \
    } \
  G_STMT_END

#define assert_interpreter_code(console, expected) \
  G_STMT_START \
    { \
      PeasInterpreter *interpreter; \
      const gchar *code; \
\
      interpreter = peas_gtk_console_get_interpreter (console); \
      code = testing_interpreter_get_code (TESTING_INTERPRETER (interpreter)); \
\
      g_assert_cmpstr (code, ==, expected); \
    } \
  G_STMT_END

G_END_DECLS

#endif /* __TESTING_CONSOLE_H__ */
