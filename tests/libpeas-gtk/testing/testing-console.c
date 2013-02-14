/*
 * testing-console.c
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

#include <string.h>

#include <gdk/gdkkeysyms.h>

#include "testing-console.h"

static void
console_foreach_cb (GtkWidget    *widget,
                    GtkTextView **text_view)
{
  if (GTK_IS_SCROLLED_WINDOW (widget))
    *text_view = GTK_TEXT_VIEW (gtk_bin_get_child (GTK_BIN (widget)));
}

GtkTextView *
testing_console_get_view (PeasGtkConsole *console)
{
  GtkTextView *text_view = NULL;

  gtk_container_foreach (GTK_CONTAINER (console),
                         (GtkCallback) console_foreach_cb,
                         &text_view);

  g_assert (GTK_IS_TEXT_VIEW (text_view));
  return text_view;
}

GtkTextBuffer *
testing_console_get_buffer (PeasGtkConsole *console)
{
  return gtk_text_view_get_buffer (testing_console_get_view (console));
}

static void
send_key (PeasGtkConsole  *console,
          guint            keyval,
          GdkModifierType  modifiers)
{
  GtkTextView *view = testing_console_get_view (console);
  GdkEvent event;
  gboolean handled;

  if (gtk_bindings_activate (G_OBJECT (view), keyval, modifiers))
    return;

  memset (&event, 0, sizeof (event));
  event.key.window = gtk_widget_get_window (GTK_WIDGET (console));
  event.key.keyval = keyval;
  event.key.state = modifiers;

  /* Assert only for key-press-event because
   * that is where they should be handled.
   */

  event.key.type = GDK_KEY_PRESS;
  g_signal_emit_by_name (view, "key-press-event", &event.key, &handled);
  g_assert (handled);

  event.key.type = GDK_KEY_RELEASE;
  g_signal_emit_by_name (view, "key-release-event", &event.key, &handled);
}

#define BINDING(modifiers, ...) \
{ \
  static guint last_key = 0; \
  static guint keys_array[] = { __VA_ARGS__ }; \
\
  G_STATIC_ASSERT (G_N_ELEMENTS (keys_array) > 0); \
\
  send_key (console, keys_array[last_key], modifiers); \
\
  if (++last_key == G_N_ELEMENTS (keys_array)) \
    last_key = 0; \
}

#define BINDING_FUNC(func, modifiers, ...) \
void \
testing_console_##func (PeasGtkConsole *console) \
BINDING (modifiers, __VA_ARGS__)

#define MOVE_BINDING_FUNC(type, ...) \
BINDING_FUNC (move_##type, 0, __VA_ARGS__) \
\
void \
testing_console_select_##type (PeasGtkConsole *console, \
                               guint           n) \
{ \
  while (n-- > 0) \
    BINDING (GDK_SHIFT_MASK, __VA_ARGS__); \
}


BINDING_FUNC (select_all, GDK_CONTROL_MASK, GDK_KEY_a)
MOVE_BINDING_FUNC (up, GDK_KEY_Up, GDK_KEY_KP_Up)
MOVE_BINDING_FUNC (down, GDK_KEY_Down, GDK_KEY_KP_Down)
MOVE_BINDING_FUNC (left, GDK_KEY_Left, GDK_KEY_KP_Left);
MOVE_BINDING_FUNC (right, GDK_KEY_Right, GDK_KEY_KP_Right)

BINDING_FUNC (move_home, 0, GDK_KEY_Home, GDK_KEY_KP_Home)
BINDING_FUNC (move_end, 0, GDK_KEY_End, GDK_KEY_KP_End)

BINDING_FUNC (backspace, 0, GDK_KEY_BackSpace)
BINDING_FUNC (complete, GDK_SUPER_MASK, GDK_KEY_space, GDK_KEY_KP_Space)
BINDING_FUNC (reset, GDK_CONTROL_MASK, GDK_KEY_d)


/* Note: don't use this to create a newline in the hope of causing
 *       the console to execute the input code instead use
 *       testing_console_execute().
 */
void
testing_console_write (PeasGtkConsole *console,
                       const gchar    *text)
{
  GtkTextBuffer *buffer = testing_console_get_buffer (console);
  GtkTextIter insert;
  GtkTextMark *insert_mark;

  insert_mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &insert, insert_mark);

  gtk_text_buffer_insert (buffer, &insert, text, -1);

  gtk_text_buffer_move_mark (buffer, insert_mark, &insert);
}

void
testing_console_execute (PeasGtkConsole *console,
                         const gchar    *code)
{
  if (code != NULL)
    testing_console_write (console, code);

  BINDING (0, GDK_KEY_Return, GDK_KEY_KP_Enter, GDK_KEY_ISO_Enter)
}

void
testing_console_set_input_pos (PeasGtkConsole *console,
                               guint           pos)
{
  GtkTextBuffer *buffer = testing_console_get_buffer (console);
  GtkTextIter iter;

  g_assert (testing_console_get_input_iter (console, &iter));

  gtk_text_iter_set_line_offset (&iter, pos);
  gtk_text_buffer_place_cursor (buffer, &iter);
}

gboolean
testing_console_get_input_iter (PeasGtkConsole *console,
                                GtkTextIter    *iter)
{
  GtkTextBuffer *buffer = testing_console_get_buffer (console);
  GtkTextIter start, input;
  gchar *prompt;
  gboolean success = FALSE;

  gtk_text_buffer_get_end_iter (buffer, &start);

  input = start;

  gtk_text_iter_set_line_offset (&start, 0);
  gtk_text_iter_set_line_offset (&input, strlen (">>> "));

  prompt = gtk_text_buffer_get_text (buffer, &start, &input, FALSE);

  if (g_strcmp0 (prompt, ">>> ") == 0 ||
      g_strcmp0 (prompt, "... ") == 0)
    {
      *iter = input;
      success = TRUE;
    }

  g_free (prompt);

  return success;
}

/* Returns the statement text with a | where the cursor is */
gchar *
testing_console_get_statement_repr (PeasGtkConsole *console)
{
  GtkTextBuffer *buffer = testing_console_get_buffer (console);
  GtkTextIter input, insert, end;
  GtkTextMark *insert_mark;
  gchar *statement;
  gint cursor_offset;
  GString *statement_repr;

  g_assert (testing_console_get_input_iter (console, &input));

  insert_mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &insert, insert_mark);

  gtk_text_buffer_get_end_iter (buffer, &end);

  cursor_offset = gtk_text_iter_get_line_offset (&insert) -
                  gtk_text_iter_get_line_offset (&input);
  g_assert (cursor_offset >= 0);

  statement = gtk_text_buffer_get_text (buffer, &input, &end, FALSE);

  statement_repr = g_string_new (statement);
  g_string_insert_c (statement_repr, cursor_offset, '|');

  g_free (statement);

  return g_string_free (statement_repr, FALSE);
}
