/*
 * testing-interpreter.c
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

#include "testing-interpreter.h"

#include <string.h>

#include <glib-object.h>

/* This is a testing interpreter.
 *
 * It's non-block prompt is ">>> " and block prompt is "... ".
 *
 * It "understands" the following commands:
 *   - "block", a block statement
 *   - "invalid", an invalid statement
 *   - "reset", causes a reset
 *   - "print <text>", writes the following text
 *   - "println <text>", writes the following text and a newline
 *   - "error <text>", writes an error with the following text
 *
 * Only executing the "invalid" command fails all others succeed.
 *
 * All commands will be added to the overall code given to the interpreter
 * except for invalid commands.
 *
 * A reset will cause the interpreter's code to be reset to "" and
 * put the interpreter back into non-block mode.
 * A reset can also be caused by the interpreter receiving a "reset" command.
 *
 * For commands leading whitespace and following text is ignored.
 * Everything else is a normal valid statement.
 *
 *
 * It can complete 3 different commands, the input must
 * start with "complete " and then be followed by:
 *
 *   - "full" -> "completed full"
 *   - "partial ", followed by:
 *     - "has-prefix" -> "pre_a_blah", "pre_b" and "pre_c_blah"
 *     - "no-prefix" -> "a_blah", "b_blah" and "c_blah"
 *   - "nothing" -> NULL
 *
 * Anything else is an invalid completion and will cause an assert.
 */

typedef struct {
  GHashTable *namespace_;

  GString *code;
  gboolean in_block;
} TestingInterpreterPrivate;

static void peas_interpreter_iface_init (PeasInterpreterInterface *iface);

G_DEFINE_TYPE_EXTENDED (TestingInterpreter,
                        testing_interpreter,
                        G_TYPE_OBJECT,
                        0,
                        G_ADD_PRIVATE (TestingInterpreter)
                        G_IMPLEMENT_INTERFACE (PEAS_TYPE_INTERPRETER,
                                               peas_interpreter_iface_init))

#define GET_PRIV(o) \
  (testing_interpreter_get_instance_private (o))

static void
value_free (gpointer value)
{
  g_boxed_free (G_TYPE_VALUE, value);
}

static void
reset_cb (PeasInterpreter    *peas_interpreter,
          TestingInterpreter *interpreter)
{
  TestingInterpreterPrivate *priv = GET_PRIV (interpreter);

  priv->in_block = FALSE;
  g_string_truncate (priv->code, 0);
}

static gchar *
testing_interpreter_prompt (PeasInterpreter *peas_interpreter)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (peas_interpreter);
  TestingInterpreterPrivate *priv = GET_PRIV (interpreter);

  if (priv->in_block)
    return g_strdup ("... ");

  return g_strdup (">>> ");
};

static gboolean
testing_interpreter_execute (PeasInterpreter *peas_interpreter,
                             const gchar     *code)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (peas_interpreter);
  TestingInterpreterPrivate *priv = GET_PRIV (interpreter);
  const gchar *command;

  g_assert (code != NULL);

  /* Skip leading whitespace */
  for (command = code; *command != '\0'; ++command)
    {
      if (!g_ascii_isspace (*command))
        break;
    }

  if (g_str_has_prefix (command, "invalid"))
    return FALSE;

  if (g_str_has_prefix (command, "reset"))
    {
      peas_interpreter_reset (peas_interpreter);
      return TRUE;
    }

  priv->in_block = g_str_has_prefix (command, "block");

  if (!priv->in_block)
    {
      if (g_str_has_prefix (command, "print "))
        {
          peas_interpreter_write (peas_interpreter,
                                  command + strlen ("print "));
        }
      else if (g_str_has_prefix (command, "println "))
        {
          peas_interpreter_write (peas_interpreter,
                                  command + strlen ("println "));
          peas_interpreter_write (peas_interpreter, "\n\n");
        }
      else if (g_str_has_prefix (command, "error "))
        {
          peas_interpreter_write_error (peas_interpreter,
                                        command + strlen ("error "));
        }
    }

  if (priv->code->len != 0)
    g_string_append_c (priv->code, '\n');

  g_string_append (priv->code, code);

  return TRUE;
}

static GList *
testing_interpreter_complete (PeasInterpreter *peas_interpreter,
                              const gchar     *code)
{
  gsize i;
  const gchar *command;
  gchar *whitespace;
  GList *completions = NULL;

  /* We add the whitespace also to the label to
   * check that the console properly displays it.
   */
#define ADD_COMPLETION(str) \
  G_STMT_START { \
    gchar *_str = g_strdup_printf ("%s" str, whitespace); \
\
    completions = g_list_append (completions, \
                                 peas_interpreter_completion_new (_str, _str)); \
  } G_STMT_END

  g_assert (code != NULL);

  for (i = 0; code[i] != '\0'; ++i)
    {
      if (!g_ascii_isspace (code[i]))
        break;
    }

  g_assert (g_str_has_prefix (code + i, "complete "));

  whitespace = g_strndup (code, i);
  command = code + i + strlen ("complete ");

  if (g_strcmp0 (command, "full") == 0)
    {
      ADD_COMPLETION ("completed full");
    }
  else if (g_strcmp0 (command, "partial has-prefix") == 0)
    {
      ADD_COMPLETION ("pre_a");
      ADD_COMPLETION ("pre_b_blah");
      ADD_COMPLETION ("pre_c");
    }
  else if (g_strcmp0 (command, "partial no-prefix") == 0)
    {
      ADD_COMPLETION ("a_blah");
      ADD_COMPLETION ("b");
      ADD_COMPLETION ("c_blah");
    }
  else if (g_strcmp0 (command, "nothing") != 0)
    {
      g_assert_not_reached ();
    }

  g_free (whitespace);

#undef ADD_COMPLETION

  return completions;
}

static const GHashTable *
testing_interpreter_get_namespace (PeasInterpreter *peas_interpreter)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (peas_interpreter);
  TestingInterpreterPrivate *priv = GET_PRIV (interpreter);

  return priv->namespace_;
}

static void
testing_interpreter_set_namespace (PeasInterpreter  *peas_interpreter,
                                   const GHashTable *namespace_)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (peas_interpreter);
  TestingInterpreterPrivate *priv = GET_PRIV (interpreter);
  GHashTableIter iter;
  const gchar *key;
  GValue *value;

  g_hash_table_remove_all (priv->namespace_);

  g_hash_table_iter_init (&iter, (GHashTable *) namespace_);
  while (g_hash_table_iter_next (&iter,
                                 (gpointer *) &key, (gpointer *) &value))
    {
      g_hash_table_insert (priv->namespace_,
                           g_strdup (key),
                           g_boxed_copy (G_TYPE_VALUE, value));
    }
}

static void
peas_interpreter_iface_init (PeasInterpreterInterface *iface)
{
  iface->prompt = testing_interpreter_prompt;
  iface->execute = testing_interpreter_execute;
  iface->complete = testing_interpreter_complete;
  iface->get_namespace = testing_interpreter_get_namespace;
  iface->set_namespace = testing_interpreter_set_namespace;
}

static void
testing_interpreter_dispose (GObject *object)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (object);
  TestingInterpreterPrivate *priv = GET_PRIV (interpreter);

  g_clear_pointer (&priv->namespace_,
                   (GDestroyNotify) g_hash_table_unref);

  if (priv->code != NULL)
    {
      g_string_free (priv->code, TRUE);
      priv->code = NULL;
    }

  G_OBJECT_CLASS (testing_interpreter_parent_class)->dispose (object);
}

static void
testing_interpreter_constructed (GObject *object)
{
  TestingInterpreter *interpreter = TESTING_INTERPRETER (object);

  g_signal_connect_object (interpreter,
                           "reset",
                           G_CALLBACK (reset_cb),
                           interpreter,
                           0);

  G_OBJECT_CLASS (testing_interpreter_parent_class)->constructed (object);
}

static void
testing_interpreter_class_init (TestingInterpreterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = testing_interpreter_dispose;
  object_class->constructed = testing_interpreter_constructed;
}

static void
testing_interpreter_init (TestingInterpreter *interpreter)
{
  TestingInterpreterPrivate *priv = GET_PRIV (interpreter);

  priv->in_block = FALSE;
  priv->code = g_string_new ("");
  priv->namespace_ = g_hash_table_new_full (g_str_hash, g_str_equal,
                                            g_free, value_free);
}

PeasInterpreter *
testing_interpreter_new (void)
{
  return PEAS_INTERPRETER (g_object_new (TESTING_TYPE_INTERPRETER, NULL));
}

const gchar *
testing_interpreter_get_code (TestingInterpreter *interpreter)
{
  TestingInterpreterPrivate *priv = GET_PRIV (interpreter);

  g_return_val_if_fail (TESTING_IS_INTERPRETER (interpreter), NULL);

  return priv->code->str;
}
