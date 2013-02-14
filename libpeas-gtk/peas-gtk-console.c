/*
 * peas-gtk-console.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "peas-gtk-console.h"

#include "peas-gtk-console-buffer.h"
#include "peas-gtk-console-history.h"

/**
 * SECTION:peas-gtk-console
 * @short_description: Console widget for interpreters.
 *
 * The #PeasGtkConsole is a widget that allows users to
 * interact with a #PeasInterpreter.
 **/

/* keypresses are handled very oddly:
 *   GtkWidget's default key-press-event: activates bindings
 *   GtkTextView has many bindings for key combos (i.e. Ctrl+a)
 *
 * So we connect the key-press-event and only handle our special bindings.
 * We also connect to GtkTextView's binding signals and override
 * them (when appropriate) to do what should be done in a console.
 */

typedef struct {
  /*GSettings *console_settings;*/
  GSettings *desktop_settings;

  PeasInterpreter *interpreter;

  GtkScrolledWindow *sw;
  GtkTextView *text_view;
  PeasGtkConsoleBuffer *buffer;

  PeasGtkConsoleHistory *history;

  /* What was entered at the prompt when history
   * was recalled (so we can restore it afterwards)
   */
  gchar *history_statement;

  guint scroll_id;
} PeasGtkConsolePrivate;

/* Properties */
enum {
  PROP_0,
  PROP_INTERPRETER,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE(PeasGtkConsole, peas_gtk_console, GTK_TYPE_BOX)

#define GET_PRIV(o) \
  (peas_gtk_console_get_instance_private (o))

/* The default monospace-font-name in gsettings-desktop-schemas */
#define DEFAULT_FONT "Monospace 11"

 /* Not implemented */
/*#define CONSOLE_SCHEMA  "org.gnome.libpeas.console"*/
#define DESKTOP_SCHEMA  "org.gnome.desktop.interface"

static void
font_settings_changed_cb (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);
  PangoFontDescription *font_desc = NULL;

  /* If a new key is used then the GSettings changed signal
   * for that key must be connected to in peas_gtk_console_init().
   */

  if (priv->desktop_settings != NULL)
    {
      gchar *font_name;

      font_name = g_settings_get_string (priv->desktop_settings,
                                         "monospace-font-name");
      font_desc = pango_font_description_from_string (font_name);

      g_free (font_name);
    }

  if (font_desc == NULL)
    font_desc = pango_font_description_from_string (DEFAULT_FONT);

  if (font_desc != NULL)
    {
      gtk_widget_override_font (GTK_WIDGET (priv->text_view),
                                font_desc);

      pango_font_description_free (font_desc);
    }
}

/* TODO: Replace with new GSettings API */
static GSettings *
settings_try_new (const gchar *schema)
{
  guint i;
  const gchar * const *schemas;

  schemas = g_settings_list_schemas ();

  if (schemas == NULL)
    return NULL;

  for (i = 0; schemas[i] != NULL; ++i)
    {
      if (g_strcmp0 (schemas[i], schema) == 0)
        return g_settings_new (schema);
    }

  return NULL;
}

static gboolean
real_scroll_to_end (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (priv->buffer);
  GtkTextIter end;

  gtk_text_buffer_get_end_iter (text_buffer, &end);

  gtk_text_buffer_place_cursor (text_buffer, &end);
  gtk_text_view_scroll_to_iter (priv->text_view,
                                &end, 0, TRUE, 0, 1);

  priv->scroll_id = 0;
  return FALSE;
}

static void
scroll_to_end (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  if (priv->scroll_id != 0)
    return;

  priv->scroll_id = g_idle_add ((GSourceFunc) real_scroll_to_end,
                                console);
}

static gint
compare_input (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (priv->buffer);
  GtkTextMark *insert_mark;
  GtkTextIter insert, input;

  insert_mark = gtk_text_buffer_get_insert (text_buffer);
  gtk_text_buffer_get_iter_at_mark (text_buffer,
                                    &insert,
                                    insert_mark);

  peas_gtk_console_buffer_get_input_iter (priv->buffer, &input);

  return gtk_text_iter_compare (&insert, &input);
}

static gboolean
selection_starts_before_input (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (priv->buffer);
  GtkTextMark *selection_bound_mark;
  GtkTextIter input, selection_bound;

  selection_bound_mark = gtk_text_buffer_get_selection_bound (text_buffer);
  gtk_text_buffer_get_iter_at_mark (text_buffer,
                                    &selection_bound,
                                    selection_bound_mark);

  peas_gtk_console_buffer_get_input_iter (priv->buffer, &input);

  return gtk_text_iter_compare (&selection_bound, &input) < 0;
}

static void
history_changed (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);
  const gchar *statement;

  if (priv->history_statement == NULL)
    {
      priv->history_statement =
            peas_gtk_console_buffer_get_statement (priv->buffer);
    }

  statement = peas_gtk_console_history_get (priv->history);

  if (statement != NULL)
    {
      peas_gtk_console_buffer_set_statement (priv->buffer, statement);
    }
  else if (priv->history_statement != NULL)
    {
      peas_gtk_console_buffer_set_statement (priv->buffer,
                                             priv->history_statement);

      g_clear_pointer (&priv->history_statement, g_free);
    }

  scroll_to_end (console);
}

static gboolean
move_left_cb (PeasGtkConsole *console,
              gboolean        extend_selection)
{
  /*GtkTextIter input;*/

  if (compare_input (console) != 0 || selection_starts_before_input (console))
    return FALSE;
/*
  peas_gtk_console_buffer_get_input_iter (priv->buffer, &input);
  peas_gtk_console_buffer_move_cursor (priv->buffer,
                                       &input,
                                       extend_selection);
*/
  return TRUE;
}

/* the following two functions should be refactored
 * the smart home/end is broken with multiple lines
 * should support the range of smart/home
 * smart end is broken goes too far to the left by 1
 */
static gboolean
move_home_cb (PeasGtkConsole *console,
              gboolean        extend_selection)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (priv->buffer);
  GtkTextMark *insert_mark;
  GtkTextIter insert, iter;

/*
  if (selection started before input)
    return FALSE;
*/

  insert_mark = gtk_text_buffer_get_insert (text_buffer);
  gtk_text_buffer_get_iter_at_mark (text_buffer, &insert, insert_mark);

  peas_gtk_console_buffer_get_input_iter (priv->buffer, &iter);

  while (g_unichar_isspace (gtk_text_iter_get_char (&iter)))
    {
      if (!gtk_text_iter_forward_char (&iter))
        break;
    }

  if (gtk_text_iter_equal (&insert, &iter))
    peas_gtk_console_buffer_get_input_iter (priv->buffer, &iter);

  peas_gtk_console_buffer_move_cursor (priv->buffer,
                                       &iter, extend_selection);

  return TRUE;
}

static gboolean
move_end_cb (PeasGtkConsole *console,
             gboolean        extend_selection)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (priv->buffer);
  GtkTextMark *insert_mark;
  GtkTextIter input, insert, iter, end;

/*
  if (selection started before input)
    return FALSE;
*/

  peas_gtk_console_buffer_get_input_iter (priv->buffer, &input);

  insert_mark = gtk_text_buffer_get_insert (text_buffer);
  gtk_text_buffer_get_iter_at_mark (text_buffer,
                                    &insert,
                                    insert_mark);

  gtk_text_buffer_get_end_iter (text_buffer, &end);
  iter = end;

  do
    {
      if (!gtk_text_iter_backward_char (&iter))
        break;
    }
  while (g_unichar_isspace (gtk_text_iter_get_char (&iter)));

  gtk_text_iter_forward_char (&iter);

  if (gtk_text_iter_compare (&iter, &input) < 0)
    gtk_text_iter_forward_char (&iter);

  if (gtk_text_iter_equal (&insert, &iter))
    gtk_text_buffer_get_end_iter (text_buffer, &iter);

  peas_gtk_console_buffer_move_cursor (priv->buffer,
                                       &iter, extend_selection);

  return TRUE;
}

static gboolean
move_up_cb (PeasGtkConsole *console,
            gboolean        extend_selection)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  if (extend_selection || compare_input (console) < 0)
    return FALSE;

  peas_gtk_console_history_next (priv->history);
  history_changed (console);

  return TRUE;
}

static gboolean
move_down_cb (PeasGtkConsole *console,
              gboolean        extend_selection)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  if (extend_selection || compare_input (console) < 0)
    return FALSE;

  /* FIXME: is this named correctly? */
  peas_gtk_console_history_previous (priv->history);
  history_changed (console);

  return TRUE;
}

static void
move_cursor_cb (GtkTextView     *text_view,
                GtkMovementStep  step,
                gint             count,
                gboolean         extend_selection,
                PeasGtkConsole  *console)
{
  gboolean handled = FALSE;

  switch (step)
    {
    case GTK_MOVEMENT_VISUAL_POSITIONS:
      if (count < 0)
        handled = move_left_cb (console, extend_selection);
      break;
    case GTK_MOVEMENT_DISPLAY_LINES:
      if (count < 0)
        handled = move_up_cb (console, extend_selection);
      else
        handled = move_down_cb (console, extend_selection);
      break;
    case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
      if (count < 0)
        handled = move_home_cb (console, extend_selection);
      else
        handled = move_end_cb (console, extend_selection);
      break;
    default:
      break;
    }

  /* No idea if we should do something special for these:
   * GTK_MOVEMENT_PARAGRAPHS, GTK_MOVEMENT_BUFFER_ENDS, GTK_MOVEMENT_PAGES
   */

  if (handled)
    g_signal_stop_emission_by_name (text_view, "move-cursor");
}

static void
move_cursor_after_cb (GtkTextView     *text_view,
                      GtkMovementStep  step,
                      gint             count,
                      gboolean         extend_selection,
                      PeasGtkConsole  *console)
{
  /* We should have an after handler for certain steps, like:
   * GTK_MOVEMENT_WORDS, GTK_MOVEMENT_HORIZONTAL_PAGES
   */

  /* - We do not care if we should extend the selection
   * - If we are not a step that we care about we do not care
   * - If we are on the input line and not after the prompt
   *   - Move to after the prompt
   */

  if (extend_selection)
    return;

  if (step != GTK_MOVEMENT_WORDS &&
      step != GTK_MOVEMENT_HORIZONTAL_PAGES)
    return;
}

static void
peas_gtk_console_complete (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  /* This is gross to use! replace with a 1.x second timeout that will
   * remove itself and if its not removed and this is reached again
   * output the completion and use tab instead of space
   */

  peas_gtk_console_buffer_write_completions (priv->buffer);
  scroll_to_end (console);
}

static void
peas_gtk_console_execute (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  gchar *statement;

  statement = peas_gtk_console_buffer_get_statement (priv->buffer);

  /* Do not add the statement if it is only
   * whitespace and remove trailing whitespace.
   */
  if (g_strchomp (statement) != NULL)
    peas_gtk_console_history_add (priv->history, statement);

  peas_gtk_console_buffer_execute (priv->buffer);

  scroll_to_end (console);

  g_free (statement);
}

static void
peas_gtk_console_reset (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  /* We emit the reset signal on the interpreter
   * so it and the console's buffer gets reset properly
   */
  g_signal_emit_by_name (priv->interpreter, "reset");
}

static void
peas_gtk_console_select_all (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (priv->buffer);
  GtkTextIter start, end;

  /* Select everything if the cusor is before the input marker or
   * a selection is already in progress and it starts before the input.
   * Otherwise just select the whole input.
   */
  if (compare_input (console) < 0 || selection_starts_before_input (console))
    {
      gtk_text_buffer_get_start_iter (text_buffer, &start);
    }
  else
    {
      peas_gtk_console_buffer_get_input_iter (priv->buffer, &start);
    }

  gtk_text_buffer_get_end_iter (text_buffer, &end);

  gtk_text_buffer_select_range (text_buffer, &start, &end);
}

static gboolean
key_press_event_cb (GtkWidget      *widget,
                    GdkEventKey    *event,
                    PeasGtkConsole *console)
{
  if (event == NULL)
    return FALSE;

  /* Would be nice to change it to a double: GDK_KEY_Tab or GDK_KEY_KP_Tab */
  if ((event->keyval == GDK_KEY_space || event->keyval == GDK_KEY_KP_Space) &&
      (event->state & GDK_SUPER_MASK) != 0)
    {
      peas_gtk_console_complete (console);
      return TRUE;
    }

  if (event->keyval == GDK_KEY_Return ||
      event->keyval == GDK_KEY_KP_Enter ||
      event->keyval == GDK_KEY_ISO_Enter)
    {
      peas_gtk_console_execute (console);
      return TRUE;
    }

  if ((event->keyval == GDK_KEY_d) &&
      (event->state & GDK_CONTROL_MASK) != 0)
    {
      peas_gtk_console_reset (console);
      return TRUE;
    }

  /* We have to do this because the "select-all" signal does not at times! */
  if ((event->keyval == GDK_KEY_a || event->keyval == GDK_KEY_slash) &&
      (event->state & GDK_CONTROL_MASK) != 0)
    {
      peas_gtk_console_select_all (console);
      return TRUE;
    }

  return FALSE;
}

static void
select_all_cb (GtkTextView    *text_view,
               gboolean        select,
               PeasGtkConsole *console)
{
  /* This is needed so the right-click menu's
   * select-all works like doing Ctrl-a
   */

  if (!select)
    return;

  peas_gtk_console_select_all (console);

  g_signal_stop_emission_by_name (text_view, "select-all");
}

static void
toggle_cursor_visible_cb (GtkTextView    *text_view,
                          PeasGtkConsole *console)
{
  /* We always want to show the cursor */
  g_signal_stop_emission_by_name (text_view, "toggle-cursor-visible");
}

static void
peas_gtk_console_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PeasGtkConsole *console = PEAS_GTK_CONSOLE (object);

  switch (prop_id)
    {
    case PROP_INTERPRETER:
      g_value_set_object (value, peas_gtk_console_get_interpreter (console));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_console_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PeasGtkConsole *console = PEAS_GTK_CONSOLE (object);
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  switch (prop_id)
    {
    case PROP_INTERPRETER:
      priv->interpreter = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_console_constructed (GObject *object)
{
  PeasGtkConsole *console = PEAS_GTK_CONSOLE (object);
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  priv->buffer = peas_gtk_console_buffer_new (priv->interpreter);
  gtk_text_view_set_buffer (priv->text_view,
                            GTK_TEXT_BUFFER (priv->buffer));

  G_OBJECT_CLASS (peas_gtk_console_parent_class)->constructed (object);
}

static void
peas_gtk_console_dispose (GObject *object)
{
  PeasGtkConsole *console = PEAS_GTK_CONSOLE (object);
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  /*g_clear_object (&priv->console_settings);*/
  g_clear_object (&priv->desktop_settings);

  g_clear_object (&priv->buffer);
  g_clear_object (&priv->interpreter);

  g_clear_pointer (&priv->history,
                   (GDestroyNotify) peas_gtk_console_history_unref);
  g_clear_pointer (&priv->history_statement, g_free);

  if (priv->scroll_id != 0)
    {
      g_source_remove (priv->scroll_id);
      priv->scroll_id = 0;
    }

  G_OBJECT_CLASS (peas_gtk_console_parent_class)->dispose (object);
}

static void
peas_gtk_console_class_init (PeasGtkConsoleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = peas_gtk_console_get_property;
  object_class->set_property = peas_gtk_console_set_property;
  object_class->constructed = peas_gtk_console_constructed;
  object_class->dispose = peas_gtk_console_dispose;

  /**
   * PeasGtkConsole:interpreter:
   *
   * The #PeasInterpreter the console is attached to.
   */
  properties[PROP_INTERPRETER] =
    g_param_spec_object ("interpreter",
                         "Interpreter",
                         "Interpreter object to execute code with",
                         PEAS_TYPE_INTERPRETER,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
peas_gtk_console_init (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  priv->history = peas_gtk_console_history_new ();

  /*priv->console_settings = g_settings_new (CONSOLE_SCHEMA);*/
  priv->desktop_settings = settings_try_new (DESKTOP_SCHEMA);

  if (priv->desktop_settings != NULL)
    g_signal_connect_swapped (priv->desktop_settings,
                              "changed::monospace-font-name",
                              (GCallback) font_settings_changed_cb,
                              console);

  priv->sw = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
  gtk_box_pack_start (GTK_BOX (console), GTK_WIDGET (priv->sw),
                      TRUE, TRUE, 0);

  /*gtk_scrolled_window_set_shadow_type (priv->sw, GTK_SHADOW_IN);*/
  gtk_scrolled_window_set_policy (priv->sw,
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  priv->text_view = GTK_TEXT_VIEW (gtk_text_view_new ());
  gtk_text_view_set_cursor_visible (priv->text_view, TRUE);
  gtk_text_view_set_wrap_mode (priv->text_view, GTK_WRAP_CHAR);
  gtk_container_add (GTK_CONTAINER (priv->sw),
                     GTK_WIDGET (priv->text_view));

  /* Cause the font to be set */
  font_settings_changed_cb (console);

  gtk_text_view_set_editable (priv->text_view, TRUE);
  gtk_text_view_set_wrap_mode (priv->text_view, GTK_WRAP_WORD_CHAR);

  /* GtkTextView already registers bindings which emit signals
   * for what they do. So we just connect and possibly
   * override the singnal handler.
   *
   * However, we still connect to "key-press-event" for our
   * special bindings like reset, complete, execute and
   * ones that GtkTextView messes up on for some reason.
   */

  g_signal_connect (priv->text_view,
                    "move-cursor",
                    G_CALLBACK (move_cursor_cb),
                    console);

  g_signal_connect_after (priv->text_view,
                          "move-cursor",
                          G_CALLBACK (move_cursor_after_cb),
                          console);

  g_signal_connect (priv->text_view,
                    "key-press-event",
                    G_CALLBACK (key_press_event_cb),
                    console);

  g_signal_connect (priv->text_view,
                    "select-all",
                    G_CALLBACK (select_all_cb),
                    console);

  g_signal_connect (priv->text_view,
                    "toggle-cursor-visible",
                    G_CALLBACK (toggle_cursor_visible_cb),
                    console);

  gtk_container_foreach (GTK_CONTAINER (console),
                         (GtkCallback) gtk_widget_show_all,
                         NULL);
}

/**
 * peas_gtk_console_new:
 * @interpreter: A #PeasInterpreter.
 *
 * Creates a new #PeasGtkConsole with @interpreter.
 *
 * Returns: the new #PeasGtkConsole object.
 */
GtkWidget *
peas_gtk_console_new (PeasInterpreter *interpreter)
{
  g_return_val_if_fail (PEAS_IS_INTERPRETER (interpreter), NULL);

  return GTK_WIDGET (g_object_new (PEAS_GTK_TYPE_CONSOLE,
                                   "interpreter", interpreter,
                                   NULL));
}

/**
 * peas_gtk_console_get_interpreter:
 * @console: a #PeasGtkConsole.
 *
 * Returns the #PeasInterpreter @console is attached to.
 *
 * Returns: (transfer none): the interpreter the console is attached to.
 */
PeasInterpreter *
peas_gtk_console_get_interpreter (PeasGtkConsole *console)
{
  PeasGtkConsolePrivate *priv = GET_PRIV (console);

  g_return_val_if_fail (PEAS_GTK_IS_CONSOLE (console), NULL);

  return priv->interpreter;
}
