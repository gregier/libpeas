/*
 * peas-gtk-console-buffer.c
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

#include "peas-gtk-console-buffer.h"

typedef struct {
  PeasInterpreter *interpreter;

  GtkTextMark *line_mark; /* What is this even for? */
  GtkTextMark *input_mark;

  GtkTextTag *uneditable_tag;
  GtkTextTag *completion_tag;

  gchar *statement;

  guint in_prompt : 1;
  guint reset_whitespace : 1;
  guint write_occured_while_in_prompt : 1;
} PeasGtkConsoleBufferPrivate;

/* Properties */
enum {
  PROP_0,
  PROP_INTERPRETER,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE(PeasGtkConsoleBuffer,
                           peas_gtk_console_buffer,
                           GTK_TYPE_TEXT_BUFFER)

#define GET_PRIV(o) \
  (peas_gtk_console_buffer_get_instance_private (o))

static gchar *
get_whitespace (const gchar *str)
{
  gsize i;

  if (str == NULL)
    return g_strdup ("");

  for (i = 0; str[i] != '\0'; ++i)
    {
      if (!g_ascii_isspace (str[i]))
        break;
    }

  return g_strndup (str, i);
}

static gchar *
get_common_string_prefix (const gchar **strings)
{
  gint i, j, k;

  for (i = 0; TRUE; ++i)
    {
      for (j = 0; strings[j] != NULL; ++j)
        {
          if (strings[j][i] == '\0')
            goto out;

          for (k = j + 1; strings[k] != NULL; ++k)
            {
              if (strings[j][i] != strings[k][i])
                goto out;
            }
        }
    }

out:

  if (i == 0)
    return NULL;

  return g_strndup (strings[j], i);
}

static void
update_statement (PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);

  if (!gtk_text_buffer_get_modified (text_buffer))
    return;

  g_clear_pointer (&priv->statement, g_free);

  if (priv->in_prompt)
    {
      GtkTextIter end, input;

      gtk_text_buffer_get_end_iter (text_buffer, &end);
      gtk_text_buffer_get_iter_at_mark (text_buffer,
                                        &input,
                                        priv->input_mark);

      priv->statement = gtk_text_buffer_get_text (text_buffer,
                                                  &input, &end,
                                                  FALSE);
    }

  gtk_text_buffer_set_modified (text_buffer, FALSE);
}

static void
write_newline (PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter end, iter;

  /* This must be before we delete the text */
  update_statement (buffer);

  gtk_text_buffer_get_end_iter (text_buffer, &end);
  peas_gtk_console_buffer_get_input_iter (buffer, &iter);

  gtk_text_buffer_delete (text_buffer, &iter, &end);

  if (priv->statement != NULL)
    {
      gtk_text_buffer_insert_with_tags (text_buffer,
                                        &end, priv->statement, -1,
                                        priv->uneditable_tag,
                                        NULL);
    }

  gtk_text_buffer_insert_with_tags (text_buffer,
                                    &end, "\n", -1,
                                    priv->uneditable_tag,
                                    NULL);

  /* Maybe this should be a diffrent mark? */
  gtk_text_buffer_move_mark (text_buffer,
                             priv->input_mark,
                             &end);
}

static void
write_text (PeasGtkConsoleBuffer *buffer,
            const gchar          *text,
            GtkTextTag           *tag)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter iter;

  if (!priv->in_prompt)
    {
      gtk_text_buffer_get_end_iter (text_buffer, &iter);
    }
  else
    {
      gtk_text_buffer_get_iter_at_mark (text_buffer,
                                        &iter,
                                        priv->input_mark);

      gtk_text_iter_set_line_offset (&iter, 0);

      if (priv->write_occured_while_in_prompt)
        gtk_text_iter_backward_char (&iter);
    }

  gtk_text_buffer_insert_with_tags (text_buffer,
                                    &iter, text, -1,
                                    priv->uneditable_tag,
                                    tag,
                                    NULL);

  if (!priv->in_prompt)
    {
      /* Should we be using a diffrent mark? */
      gtk_text_buffer_move_mark (text_buffer,
                                 priv->input_mark,
                                 &iter);
    }
  else if (!priv->write_occured_while_in_prompt)
    {
      priv->write_occured_while_in_prompt = TRUE;
      gtk_text_buffer_insert_with_tags (text_buffer,
                                        &iter, "\n", -1,
                                        priv->uneditable_tag,
                                        NULL);
    }
}

static void
interpreter_write_cb (PeasInterpreter      *interpreter,
                      const gchar          *text,
                      PeasGtkConsoleBuffer *buffer)
{
  if (text == NULL)
    return;

  write_text (buffer, text, NULL);
}

static void
interpreter_write_error_cb (PeasInterpreter      *interpreter,
                            const gchar          *text,
                            PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);

  priv->reset_whitespace = TRUE;

  if (text == NULL)
    return;

  /* Should we pass a colored tag here? */
  write_text (buffer, text, NULL);
}

static void
interpreter_reset_cb (PeasInterpreter      *interpreter,
                      PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter start, end;

  gtk_text_buffer_get_start_iter (text_buffer, &start);
  gtk_text_buffer_get_end_iter (text_buffer, &end);

  gtk_text_buffer_delete (text_buffer, &start, &end);

  gtk_text_buffer_move_mark (text_buffer,
                             priv->line_mark,
                             &end);
  gtk_text_buffer_move_mark (text_buffer,
                             priv->input_mark,
                             &end);

  peas_gtk_console_buffer_write_prompt (buffer);
}

static void
peas_gtk_console_buffer_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PeasGtkConsoleBuffer *buffer = PEAS_GTK_CONSOLE_BUFFER (object);
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);

  switch (prop_id)
    {
    case PROP_INTERPRETER:
      g_value_set_object (value, priv->interpreter);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_console_buffer_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PeasGtkConsoleBuffer *buffer = PEAS_GTK_CONSOLE_BUFFER (object);
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);

  switch (prop_id)
    {
    case PROP_INTERPRETER:
      priv->interpreter = g_object_ref (g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_gtk_console_buffer_constructed (GObject *object)
{
  PeasGtkConsoleBuffer *buffer = PEAS_GTK_CONSOLE_BUFFER (object);
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter end;
  const GHashTable *namespace_;

  gtk_text_buffer_set_modified (text_buffer, FALSE);
  gtk_text_buffer_get_end_iter (text_buffer, &end);

  priv->line_mark = gtk_text_buffer_create_mark (text_buffer,
                                                         NULL, &end, TRUE);
  priv->input_mark = gtk_text_buffer_create_mark (text_buffer,
                                                          NULL, &end, TRUE);

  priv->uneditable_tag =
      gtk_text_buffer_create_tag (text_buffer, NULL,
                                  "editable", FALSE,
                                  "wrap-mode", GTK_WRAP_CHAR,
                                  NULL);
  priv->completion_tag =
      gtk_text_buffer_create_tag (text_buffer, NULL,
                                  "wrap-mode", GTK_WRAP_WORD_CHAR,
                                  NULL);

  g_signal_connect_object (priv->interpreter,
                           "write",
                           G_CALLBACK (interpreter_write_cb),
                           buffer,
                           0);
  g_signal_connect_object (priv->interpreter,
                           "write-error",
                           G_CALLBACK (interpreter_write_error_cb),
                           buffer,
                           0);

  /* We need to get the updated prompt which is only
   * available after the interpreter has been reset.
   */
  g_signal_connect_object (priv->interpreter,
                           "reset",
                           G_CALLBACK (interpreter_reset_cb),
                           buffer,
                           G_CONNECT_AFTER);

  namespace_ = peas_interpreter_get_namespace (priv->interpreter);

  if (namespace_ != NULL)
    {
      GHashTableIter namespace_iter;
      const gchar *namespace_key;

      g_hash_table_iter_init (&namespace_iter, (GHashTable *) namespace_);

      if (g_hash_table_iter_next (&namespace_iter,
                                  (gpointer) &namespace_key, NULL))
        {
          GString *string;

          if (g_hash_table_size ((GHashTable *) namespace_) == 1)
            {
              string = g_string_new (_("Predefined variable: "));
            }
          else
            {
              string = g_string_new (_("Predefined variables: "));
            }

          g_string_append_printf (string, "'%s'", namespace_key);

          while (g_hash_table_iter_next (&namespace_iter,
                                         (gpointer) &namespace_key, NULL))
            {
              g_string_append_printf (string, ", '%s'", namespace_key);
            }

          write_text (buffer, string->str, NULL);
          g_string_free (string, TRUE);
        }
    }

  /* Show the prompt */
  peas_gtk_console_buffer_write_prompt (buffer);

  G_OBJECT_CLASS (peas_gtk_console_buffer_parent_class)->constructed (object);
}

static void
peas_gtk_console_buffer_dispose (GObject *object)
{
  PeasGtkConsoleBuffer *buffer = PEAS_GTK_CONSOLE_BUFFER (object);
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);

  g_clear_pointer (&priv->statement, g_free);
  g_clear_object (&priv->interpreter);
}

static void
peas_gtk_console_buffer_class_init (PeasGtkConsoleBufferClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = peas_gtk_console_buffer_get_property;
  object_class->set_property = peas_gtk_console_buffer_set_property;
  object_class->constructed = peas_gtk_console_buffer_constructed;
  object_class->dispose = peas_gtk_console_buffer_dispose;

  /**
   * PeasGtkBuffer:interpreter:
   *
   * The #PeasInterpreter the buffer is attached to.
   */
  properties[PROP_INTERPRETER] =
    g_param_spec_object ("interpreter",
                         "Interpreter",
                         "Interpreter",
                         PEAS_TYPE_INTERPRETER,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
peas_gtk_console_buffer_init (PeasGtkConsoleBuffer *buffer)
{
}

/**
 * peas_gtk_console_buffer_new:
 * @interpreter: A #PeasInterpreter.
 *
 * Creates a new #PeasGtkConsoleBuffer with the given interpreter.
 *
 * Returns: a new #PeasGtkConsoleBuffer object.
 */
PeasGtkConsoleBuffer *
peas_gtk_console_buffer_new (PeasInterpreter *interpreter)
{
  g_return_val_if_fail (PEAS_IS_INTERPRETER (interpreter), NULL);

  return PEAS_GTK_CONSOLE_BUFFER (g_object_new (PEAS_GTK_TYPE_CONSOLE_BUFFER,
                                                "interpreter", interpreter,
                                                NULL));
}

/**
 * peas_gtk_console_buffer_get_interpreter:
 * @buffer: a #PeasGtkConsoleBuffer.
 *
 * Returns the #PeasInterpreter @buffer is attached to.
 *
 * Returns: the interpreter the buffer is attached to.
 */
PeasInterpreter *
peas_gtk_console_buffer_get_interpreter (PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);

  g_return_val_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer), NULL);

  return priv->interpreter;
}

/**
 * peas_gtk_console_buffer_execute:
 * @buffer: A #PeasGtkConsoleBuffer.
 *
 * Executes the current statement.
 */
void
peas_gtk_console_buffer_execute (PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  gchar *whitespace;
  gboolean success;

  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));
  g_return_if_fail (priv->in_prompt);

  update_statement (buffer);

  whitespace = get_whitespace (priv->statement);

  priv->in_prompt = FALSE;
  priv->reset_whitespace = FALSE;
  priv->write_occured_while_in_prompt = FALSE;

  write_newline (buffer);

  success = peas_interpreter_execute (priv->interpreter,
                                      priv->statement);

  /* Reset also resets the whitespace */
  if (priv->in_prompt)
    priv->reset_whitespace = TRUE;

  /* Do not cause a double prompt if reset was emitted */
  if (!priv->in_prompt)
    peas_gtk_console_buffer_write_prompt (buffer);

  if (success && !priv->reset_whitespace)
    peas_gtk_console_buffer_set_statement (buffer, whitespace);

  g_free (whitespace);
}

/**
 * peas_gtk_console_buffer_write_prompt:
 * @buffer: a #PeasGtkConsoleBuffer.
 *
 * Writes the interpreter's prompt to the buffer.
 */
void
peas_gtk_console_buffer_write_prompt (PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
  GtkTextIter input, end;
  gchar *prompt;

  /* should this call set_modified? */

  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));

  prompt = peas_interpreter_prompt (priv->interpreter);

  /* Is this enough to always be correctly on a newline? */
  peas_gtk_console_buffer_get_input_iter (buffer, &input);
  if (!gtk_text_iter_starts_line (&input))
    write_newline (buffer);

  gtk_text_buffer_get_end_iter (text_buffer, &end);

  if (prompt != NULL)
    {
      gtk_text_buffer_insert_with_tags (text_buffer,
                                        &end, prompt, -1,
                                        priv->uneditable_tag,
                                        NULL);
    }

  gtk_text_buffer_move_mark (text_buffer,
                             priv->input_mark,
                             &end);

  priv->in_prompt = TRUE;

  g_free (prompt);
}

static const gchar *
skip_whitespace (const gchar *str)
{
  while (str != NULL && g_ascii_isspace (*str))
    ++str;

  return str;
}

/**
 * peas_gtk_console_buffer_write_completions:
 * @buffer: A #PeasGtkConsoleBuffer.
 *
 * Writes the interpreter's completions to the buffer
 * and updates the statement to reflect the completions.
 *
 * Note: it is an error to call this if @buffer is not in prompt mode.
 */
void
peas_gtk_console_buffer_write_completions (PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  gchar *whitespace;
  GString *new_statement;
  GList *completions;
  const gchar *str;

  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));
  g_return_if_fail (priv->in_prompt);

  update_statement (buffer);

  if (priv->statement == NULL)
    priv->statement = g_strdup ("");

  /* Do not strip the statement when sending it for completion */
  completions = peas_interpreter_complete (priv->interpreter,
                                           priv->statement);

  if (completions == NULL)
    return;

  whitespace = get_whitespace (priv->statement);
  new_statement = g_string_new (whitespace);

  if (completions->next == NULL)
    {
      str = peas_interpreter_completion_get_text (completions->data);
      g_string_append (new_statement, skip_whitespace (str));
    }
  else
    {
#if 1
      GArray *data;
      GList *completion;
      guint n_completions;
      gchar **texts;
      gsize i;
      gsize len;
      gsize max_len = 0;
      gchar *filler;
      gchar *prefix;

      data = g_array_new (FALSE, FALSE, sizeof (gsize));

      n_completions = g_list_length (completions);
      texts = g_new (gchar *, n_completions + 1);

      for (completion = completions, i = 0; completion != NULL;
           completion = completion->next)
        {
          str = peas_interpreter_completion_get_label (completion->data);
          str = skip_whitespace (str);

          if (str == NULL || *str == '\0')
            continue;

          len = strlen (str);
          g_array_append_val (data, len);

          if (len > max_len)
            max_len = len;

          str = peas_interpreter_completion_get_text (completion->data);
          texts[i++] = (gchar *) skip_whitespace (str);
        }

      texts[i] = NULL;

      filler = g_strnfill (max_len, ' ');

      priv->in_prompt = FALSE;
      write_newline (buffer);

      for (completion = completions, i = 0; completion != NULL;
           completion = completion->next)
        {
          str = peas_interpreter_completion_get_label (completion->data);
          str = skip_whitespace (str);

          if (str == NULL || *str == '\0')
            continue;

          if (i > 0)
            write_text (buffer, filler + len - 1, NULL);

          len = g_array_index (data, gsize, i++);

          write_text (buffer, str, priv->completion_tag);
        }

      /* Go back to prompt mode */
      peas_gtk_console_buffer_write_prompt (buffer);

      prefix = get_common_string_prefix ((const gchar **) texts);

      if (prefix != NULL)
        g_string_append (new_statement, prefix);

      g_free (prefix);
      g_free (filler);
      g_free (texts);
      g_array_unref (data);
#else
      guint n_completions;
      GString *completions_str;
      gchar **texts;
      GList *completion;
      guint i = 0;
      gchar *prefix;

      n_completions = g_list_length (completions);

      completions_str = g_string_sized_new (5 * n_completions);
      texts = g_new (gchar *, n_completions + 1);

      for (completion = completions; completion != NULL;
           completion = completion->next)
        {
          str = peas_interpreter_completion_get_label (completion->data);
          str = skip_whitespace (str);

          if (str == NULL || *str == '\0')
            continue;

          if (completions_str->len != 0)
            g_string_append_c (completions_str, ' ');

          g_string_append (completions_str, str);

          str = peas_interpreter_completion_get_text (completion->data);
          texts[i++] = (gchar *) skip_whitespace (str);
        }

      texts[i] = NULL;

      /* Write the completions after the prompt */
      priv->in_prompt = FALSE;
      write_newline (buffer);

      write_text (buffer, completions_str->str, NULL);

      /* Go back to prompt mode */
      peas_gtk_console_buffer_write_prompt (buffer);

      prefix = get_common_string_prefix ((const gchar **) texts);

      if (prefix != NULL)
        g_string_append (new_statement, prefix);

      g_free (prefix);
      g_free (texts);
      g_string_free (completions_str, TRUE);
#endif
    }

  peas_gtk_console_buffer_set_statement (buffer, new_statement->str);

  g_string_free (new_statement, TRUE);
  g_free (whitespace);

  g_list_free_full (completions, g_object_unref);
}
/**
 * peas_gtk_console_buffer_get_statement:
 * @buffer: A #PeasGtkConsoleBuffer.
 *
 * Returns the current statement or %NULL if not currently in prompt mode.
 *
 * Returns: the current statement.
 */
gchar *
peas_gtk_console_buffer_get_statement (PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);

  g_return_val_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer), NULL);

  update_statement (buffer);

  /* Weird to return a copy of the string
   * but it reduces errors because the
   * statement can change behind your back
   * quite often and thus cause segfaults.
   *
   * This is also what GtkTextBuffer does.
   */
  return g_strdup (priv->statement);
}

/**
 * peas_gtk_console_buffer_set_statement:
 * @buffer: A #PeasGtkConsoleBuffer.
 * @statement: text to set as the current statement.
 *
 * Replaces the current statement with @statement.
 *
 * Note: it is an error to call this if @buffer is not in prompt mode.
 */
void
peas_gtk_console_buffer_set_statement (PeasGtkConsoleBuffer *buffer,
                                       const gchar          *statement)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);
  GtkTextBuffer *text_buffer;
  GtkTextIter end, input;

  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));
  g_return_if_fail (statement != NULL);
  g_return_if_fail (priv->in_prompt);

  text_buffer = GTK_TEXT_BUFFER (buffer);

  if (priv->statement != NULL)
    g_free (priv->statement);

  gtk_text_buffer_get_end_iter (text_buffer, &end);
  peas_gtk_console_buffer_get_input_iter (buffer, &input);

  gtk_text_buffer_delete (text_buffer, &input, &end);
  gtk_text_buffer_insert (text_buffer, &end, statement, -1);

  priv->statement = g_strdup (statement);

  gtk_text_buffer_set_modified (text_buffer, FALSE);
}

/**
 * peas_gtk_console_buffer_get_input_mark:
 * @buffer: A #PeasGtkConsoleBuffer.
 *
 * Returns the input mark associated
 * with the starting position for input.
 *
 * Returns: the input mark.
 */
GtkTextMark *
peas_gtk_console_buffer_get_input_mark (PeasGtkConsoleBuffer *buffer)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);

  g_return_val_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer), NULL);

  return priv->input_mark;
}

/**
 * peas_gtk_console_buffer_get_input_iter:
 * @buffer: A #PeasGtkConsoleBuffer.
 * @iter: A #GtkTextIter.
 *
 * Sets @iter to the starting position for input.
 */
void
peas_gtk_console_buffer_get_input_iter (PeasGtkConsoleBuffer *buffer,
                                        GtkTextIter          *iter)
{
  PeasGtkConsoleBufferPrivate *priv = GET_PRIV (buffer);

  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));
  g_return_if_fail (iter != NULL);

  gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
                                    iter,
                                    priv->input_mark);
}

/**
 * peas_gtk_console_buffer_move_cursor:
 * @buffer: A #PeasGtkConsoleBuffer.
 * @iter: A #GtkTextIter.
 * @extend_selection: %TRUE if the move should extend the selection.
 *
 * Move the cursor according the @extend_selection.
 * If %TRUE then the selection is expanded to
 * @iter and if not the cursor is directly moved to @iter.
 */
void
peas_gtk_console_buffer_move_cursor (PeasGtkConsoleBuffer *buffer,
                                     GtkTextIter          *iter,
                                     gboolean              extend_selection)
{
  GtkTextBuffer *text_buffer;

  g_return_if_fail (PEAS_GTK_IS_CONSOLE_BUFFER (buffer));
  g_return_if_fail (iter != NULL);

  text_buffer = GTK_TEXT_BUFFER (buffer);

  if (!extend_selection)
    {
      gtk_text_buffer_place_cursor (text_buffer, iter);
    }
  else
    {
      GtkTextMark *insert;

      insert = gtk_text_buffer_get_insert (text_buffer);
      gtk_text_buffer_move_mark (text_buffer, insert, iter);
    }
}
