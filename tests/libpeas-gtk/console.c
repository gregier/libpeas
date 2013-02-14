/*
 * console.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "testing/testing.h"
#include "testing/testing-console.h"
#include "testing/testing-interpreter.h"

typedef struct _TestFixture TestFixture;

struct _TestFixture {
  GtkWidget *window;
  PeasGtkConsole *console;
};

static void
test_setup (TestFixture   *fixture,
            gconstpointer  data)
{
  PeasInterpreter *interpreter;

  interpreter = testing_interpreter_new ();
  g_assert (TESTING_IS_INTERPRETER (interpreter));

  fixture->console = PEAS_GTK_CONSOLE (peas_gtk_console_new (interpreter));
  g_assert (PEAS_GTK_IS_CONSOLE (fixture->console));

  /* GtkTextView needs a window to work properly */
  fixture->window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_container_add (GTK_CONTAINER (fixture->window),
                     GTK_WIDGET (fixture->console));
  gtk_widget_set_size_request (fixture->window, 800, 600);
  gtk_widget_show_all (fixture->window);
  gtk_widget_hide (fixture->window);

  g_object_unref (interpreter);
}

static void
test_teardown (TestFixture   *fixture,
               gconstpointer  data)
{
  gtk_widget_destroy (GTK_WIDGET (fixture->window));
}

static void
test_runner (TestFixture   *fixture,
             gconstpointer  data)
{
  ((void (*) (PeasGtkConsole *)) data) (fixture->console);
}

static void
test_gtk_console_prompt_nonblock (PeasGtkConsole *console)
{
  assert_console_text (console, ">>> ");
}

static void
test_gtk_console_prompt_block (PeasGtkConsole *console)
{
  testing_console_execute (console, "block");

  assert_console_text (console, ">>> block\n... ");
}

static void
test_gtk_console_prompt_write (PeasGtkConsole *console)
{
  PeasInterpreter *interpreter;

  interpreter = peas_gtk_console_get_interpreter (console);

  /* Do not execute so we can test with a newline above us and without */
  testing_console_write (console, "a");

  g_signal_emit_by_name (interpreter, "write", "b");
  assert_console_text (console, "b\n>>> a");

  g_signal_emit_by_name (interpreter, "write", "c\n");
  assert_console_text (console, "bc\n\n>>> a");

  g_signal_emit_by_name (interpreter, "write", "d\n");
  assert_console_text (console, "bc\nd\n\n>>> a");
}

static void
test_gtk_console_namespace (void)
{
  PeasInterpreter *interpreter;
  GHashTable *namespace_;
  PeasGtkConsole *console;
  GValue values[2] = { { 0 }, { 0 } };

  interpreter = testing_interpreter_new ();
  namespace_ = g_hash_table_new (g_str_hash, g_str_equal);

  g_value_init (&values[0], G_TYPE_INT);
  g_value_set_int (&values[0], 1);
  g_hash_table_insert (namespace_, (gpointer) "a", &values[0]);

  peas_interpreter_set_namespace (interpreter, namespace_);
  console = PEAS_GTK_CONSOLE (peas_gtk_console_new (interpreter));
  assert_console_text (console, "Predefined variable: 'a'\n>>> ");
  gtk_widget_destroy (GTK_WIDGET (console));


  g_value_init (&values[1], G_TYPE_INT);
  g_value_set_int (&values[1], 1);
  g_hash_table_insert (namespace_, (gpointer) "b", &values[1]);

  peas_interpreter_set_namespace (interpreter, namespace_);
  console = PEAS_GTK_CONSOLE (peas_gtk_console_new (interpreter));
  assert_console_text (console, "Predefined variables: 'a', 'b'\n>>> ");
  gtk_widget_destroy (GTK_WIDGET (console));

  g_hash_table_unref (namespace_);
  g_object_unref (interpreter);
}

static void
test_gtk_console_execute_valid (PeasGtkConsole *console)
{
  testing_console_execute (console, "a valid statement");

  assert_console_text (console, ">>> a valid statement\n>>> ");
  assert_interpreter_code (console, "a valid statement");
}

static void
test_gtk_console_execute_invalid (PeasGtkConsole *console)
{
  testing_console_execute (console, "invalid");

  assert_console_text (console, ">>> invalid\n>>> ");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_execute_reset (PeasGtkConsole *console)
{
  /* Make sure the reset actually clears the buffer */
  testing_console_execute (console, "a statement");
  testing_console_execute (console, "reset");

  assert_console_text (console, ">>> ");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_execute_write (PeasGtkConsole *console)
{
  testing_console_execute (console, "print Hello, World!");

  assert_console_text (console, ">>> print Hello, World!\nHello, World!\n>>> ");
  assert_interpreter_code (console, "print Hello, World!");
}

static void
test_gtk_console_execute_write_newline (PeasGtkConsole *console)
{
  testing_console_execute (console, "println Hello, World!");

  assert_console_text (console, ">>> println Hello, World!\nHello, World!\n\n>>> ");
  assert_interpreter_code (console, "println Hello, World!");
}

static void
test_gtk_console_execute_write_error (PeasGtkConsole *console)
{
  testing_console_execute (console, "error Error Message");

  assert_console_text (console, ">>> error Error Message\n"
                                "Error Message\n>>> ");
  assert_interpreter_code (console, "error Error Message");


  testing_console_reset (console);


  testing_console_execute (console, "  error Error Message");

  assert_console_text (console, ">>>   error Error Message\n"
                                "Error Message\n>>> ");
  assert_interpreter_code (console, "  error Error Message");
}

static void
test_gtk_console_execute_with_whitespace_valid (PeasGtkConsole *console)
{
  testing_console_execute (console, "  I have whitespace");

  assert_console_text (console, ">>>   I have whitespace\n>>>   ");
  assert_interpreter_code (console, "  I have whitespace");
}

static void
test_gtk_console_execute_with_whitespace_invalid (PeasGtkConsole *console)
{
  testing_console_execute (console, "  invalid I have whitespace");

  assert_console_text (console, ">>>   invalid I have whitespace\n>>> ");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_execute_with_whitespace_reset (PeasGtkConsole *console)
{
  /* We use whitespace here to make sure the reset
   * does not add it back to the input after the reset.
   */
  testing_console_execute (console, "  the reset will delete this");
  testing_console_execute (console, "  reset");

  assert_console_text (console, ">>> ");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_keybindings_execute (PeasGtkConsole *console)
{
  testing_console_execute (console, "a");
  testing_console_execute (console, "b");
  testing_console_execute (console, "c");

  assert_console_text (console, ">>> a\n>>> b\n>>> c\n>>> ");
  assert_interpreter_code (console, "a\nb\nc");
}

/* For many of these tests things may be repeated
 * to make sure that ALL of the keybindings work.
 */

static void
test_gtk_console_keybindings_backspace (PeasGtkConsole *console)
{
  testing_console_write (console, "a");

  testing_console_backspace (console);
  /* Prompt should be uneditable */
  testing_console_backspace (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> \n>>> ");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_keybindings_left (PeasGtkConsole *console)
{
  /* Should not be able to move into the prompt */
  testing_console_move_left (console);

  testing_console_write (console, "a");

  testing_console_move_left (console);

  testing_console_write (console, "b");

  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> ba\n>>> ");
  assert_interpreter_code (console, "ba");
}

static void
test_gtk_console_keybindings_right (PeasGtkConsole *console)
{
  /* I know this is dumb but this has broken before */

  testing_console_move_right (console);

  testing_console_write (console, "a");

  testing_console_move_left (console);
  testing_console_move_right (console);

  testing_console_write (console, "b");

  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> ab\n>>> ");
  assert_interpreter_code (console, "ab");
}

static void
test_gtk_console_keybindings_reset (PeasGtkConsole *console)
{
  /* We use whitespace here to make sure the reset
   * does not add it back to the input after the reset.
   */
  testing_console_execute (console, "  the reset will delete this");

  testing_console_reset (console);

  assert_console_text (console, ">>> ");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_keybindings_complete_full (PeasGtkConsole *console)
{
  testing_console_write (console, "complete full");

  testing_console_complete (console);

  assert_console_text (console, ">>> completed full");
  assert_interpreter_code (console, "");


  testing_console_reset (console);


  testing_console_write (console, "  complete full");

  testing_console_complete (console);

  assert_console_text (console, ">>>   completed full");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_keybindings_complete_has_prefix (PeasGtkConsole *console)
{
  /* By completing pre_a, pre_b_blah and pre_c
   * it makes the prefix finder have to special
   * case that in completes a whole string.
   */

  /* The extra space after the 'a' when
   * completing is due to the column wrapping.
   * Note: extra space is not added after last completion.
   */

  testing_console_write (console, "complete partial has-prefix");

  testing_console_complete (console);

  assert_console_text (console, ">>> complete partial has-prefix\n"
                                "pre_a      pre_b_blah pre_c\n"
                                ">>> pre_");
  assert_interpreter_code (console, "");


  testing_console_reset (console);


  testing_console_write (console, "  complete partial has-prefix");

  testing_console_complete (console);

  assert_console_text (console, ">>>   complete partial has-prefix\n"
                                "pre_a      pre_b_blah pre_c\n"
                                ">>>   pre_");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_keybindings_complete_no_prefix (PeasGtkConsole *console)
{
  /* The extra space after the 'b' when
   * completing is due to the column wrapping.
   * Note: extra space is not added after last completion.
   */

  testing_console_write (console, "complete partial no-prefix");

  testing_console_complete (console);

  assert_console_text (console, ">>> complete partial no-prefix\n"
                                "a_blah b      c_blah\n"
                                ">>> ");
  assert_interpreter_code (console, "");


  testing_console_reset (console);


  testing_console_write (console, "  complete partial no-prefix");

  testing_console_complete (console);

  assert_console_text (console, ">>>   complete partial no-prefix\n"
                                "a_blah b      c_blah\n"
                                ">>>   ");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_keybindings_complete_nothing (PeasGtkConsole *console)
{
  testing_console_write (console, "complete nothing");

  testing_console_complete (console);

  assert_console_text (console, ">>> complete nothing");
  assert_interpreter_code (console, "");
}

static void
test_gtk_console_keybindings_home_nonsmart (PeasGtkConsole *console)
{
  gint i;

  testing_console_move_home (console);
  assert_console_cursor_pos (console, "|");

  for (i = 0; i < 2; ++i)
    {
      testing_console_write (console, "a statement");

      testing_console_move_home (console);
      assert_console_cursor_pos (console, "|a statement");

      testing_console_execute (console, NULL);
    }
}

static void
test_gtk_console_keybindings_home_smart (PeasGtkConsole *console)
{
  testing_console_write (console, "  I have whitespace");

  testing_console_move_home (console);
  assert_console_cursor_pos (console, "  |I have whitespace");

  testing_console_move_home (console);
  assert_console_cursor_pos (console, "|  I have whitespace");

  testing_console_move_home (console);
  assert_console_cursor_pos (console, "  |I have whitespace");
}

static void
test_gtk_console_keybindings_end_nonsmart (PeasGtkConsole *console)
{
  gint i;

  testing_console_move_end (console);
  assert_console_cursor_pos (console, "|");

  for (i = 0; i < 2; ++i)
    {
      testing_console_write (console, "a statement");

      testing_console_move_end (console);
      assert_console_cursor_pos (console, "a statement|");

      testing_console_execute (console, NULL);
    }
}

static void
test_gtk_console_keybindings_end_smart (PeasGtkConsole *console)
{
  testing_console_write (console, "I have whitespace  ");

  testing_console_move_end (console);
  assert_console_cursor_pos (console, "I have whitespace|  ");

  testing_console_move_end (console);
  assert_console_cursor_pos (console, "I have whitespace  |");

  testing_console_move_end (console);
  assert_console_cursor_pos (console, "I have whitespace|  ");
}

static void
test_gtk_console_selection_inside_input (PeasGtkConsole *console)
{
  testing_console_write (console, "blah");

  testing_console_select_left (console, strlen (">>> blah"));

  assert_console_selection (console, "blah");
}

static void
test_gtk_console_selection_outside_input (PeasGtkConsole *console)
{
  testing_console_write (console, "blah");

  testing_console_set_input_pos (console, 0);
  testing_console_select_right (console, strlen (">>> blah"));
  assert_console_selection (console, ">>> blah");

  /* Check that the start of the selection is taken into account */
  testing_console_select_left (console, strlen ("ah"));
  assert_console_selection (console, ">>> bl");

  testing_console_select_left (console, strlen (" bl"));
  assert_console_selection (console, ">>>");

  testing_console_select_left (console, strlen (">>>"));
  assert_console_selection (console, NULL);
}

static void
test_gtk_console_selection_select_all (PeasGtkConsole *console)
{
  testing_console_write (console, "blah");

  testing_console_select_all (console);
  assert_console_selection (console, "blah");

  testing_console_set_input_pos (console, 0);

  testing_console_select_all (console);
  assert_console_selection (console, ">>> blah");
}

static void
test_gtk_console_selection_up (PeasGtkConsole *console)
{
  testing_console_execute (console, "blah");

  testing_console_select_up (console, 1);

  assert_console_selection (console, "blah\n>>> ");

  /* Make sure the left and right keys behave correctly */
  testing_console_select_left (console, strlen (">>> "));
  testing_console_select_right (console, strlen (">"));

  assert_console_selection (console, ">> blah\n>>> ");
}

static void
test_gtk_console_selection_down (PeasGtkConsole *console)
{
  testing_console_execute (console, "blah");
  testing_console_execute (console, "blah");

  testing_console_select_up (console, 2);
  testing_console_select_down (console, 1);

  assert_console_selection (console, "blah\n>>> ");

  /* Make sure the left and right keys behave correctly */
  testing_console_select_left (console, strlen (">>> "));
  testing_console_select_right (console, strlen (">"));

  assert_console_selection (console, ">> blah\n>>> ");
}

static void
test_gtk_console_history_up_empty (PeasGtkConsole *console)
{
  testing_console_write (console, "a statement");

  testing_console_move_up (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> a statement\n>>> ");
  assert_interpreter_code (console, "a statement");
}

static void
test_gtk_console_history_up_nonempty (PeasGtkConsole *console)
{
  testing_console_execute (console, "a statement");

  testing_console_move_up (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> a statement\n>>> a statement\n>>> ");
  assert_interpreter_code (console, "a statement\na statement");
}

static void
test_gtk_console_history_up_past_top (PeasGtkConsole *console)
{
  testing_console_execute (console, "a statement");

  testing_console_move_up (console);
  testing_console_move_up (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> a statement\n>>> a statement\n>>> ");
  assert_interpreter_code (console, "a statement\na statement");
}

static void
test_gtk_console_history_down_empty (PeasGtkConsole *console)
{
  testing_console_write (console, "a statement");

  testing_console_move_down (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> a statement\n>>> ");
  assert_interpreter_code (console, "a statement");
}

static void
test_gtk_console_history_down_nonempty (PeasGtkConsole *console)
{
  testing_console_execute (console, "a statement");
  testing_console_execute (console, "another statement");

  testing_console_move_up (console);
  testing_console_move_up (console);
  testing_console_move_down (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> a statement\n>>> another statement\n"
                                ">>> another statement\n>>> ");
  assert_interpreter_code (console, "a statement\nanother statement\n"
                                    "another statement");
}

static void
test_gtk_console_history_restore_empty (PeasGtkConsole *console)
{
  testing_console_write (console, "a statement");

  testing_console_move_up (console);
  testing_console_move_down (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> a statement\n>>> ");
  assert_interpreter_code (console, "a statement");
}

static void
test_gtk_console_history_restore_nonempty (PeasGtkConsole *console)
{
  testing_console_execute (console, "a statement");

  testing_console_write (console, "restored statement");

  testing_console_move_up (console);
  testing_console_move_down (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> a statement\n>>> restored statement\n>>> ");
  assert_interpreter_code (console, "a statement\nrestored statement");
}

static void
test_gtk_console_history_restore_past_end (PeasGtkConsole *console)
{
  testing_console_execute (console, "a statement");

  testing_console_write (console, "restored statement");

  testing_console_move_up (console);
  testing_console_move_down (console);
  testing_console_move_down (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> a statement\n>>> restored statement\n>>> ");
  assert_interpreter_code (console, "a statement\nrestored statement");
}

static void
test_gtk_console_history_trailing_whitespace (PeasGtkConsole *console)
{
  testing_console_execute (console, "I have whitespace  ");

  testing_console_move_up (console);
  testing_console_execute (console, NULL);

  assert_console_text (console, ">>> I have whitespace  \n"
                                ">>> I have whitespace\n"
                                ">>> ");
}

int
main (int   argc,
      char *argv[])
{
  testing_init (&argc, &argv);

#define TEST(path, ftest) \
  g_test_add ("/gtk/console/" path, TestFixture, \
              (gpointer) test_gtk_console_##ftest, \
              test_setup, test_runner, test_teardown)

#define TEST_FUNC(path, ftest) \
  g_test_add_func ("/gtk/console/" path, (gpointer) test_gtk_console_##ftest)

  TEST ("prompt/nonblock", prompt_nonblock);
  TEST ("prompt/block", prompt_block);
  TEST ("prompt/write", prompt_write);

  TEST ("namespace", namespace);

  TEST ("execute/valid", execute_valid);
  TEST ("execute/invalid", execute_invalid);
  TEST ("execute/reset", execute_reset);
  TEST ("execute/write", execute_write);
  TEST ("execute/write-newline", execute_write_newline);
  TEST ("execute/write-error", execute_write_error);
  TEST ("execute/with-whitespace/valid", execute_with_whitespace_valid);
  TEST ("execute/with-whitespace/invalid", execute_with_whitespace_invalid);
  TEST ("execute/with-whitespace/reset", execute_with_whitespace_reset);

  TEST ("keybindings/execute", keybindings_execute);
  TEST ("keybindings/backspace", keybindings_backspace);
  TEST ("keybindings/left", keybindings_left);
  TEST ("keybindings/right", keybindings_right);
  TEST ("keybindings/reset", keybindings_reset);
  TEST ("keybindings/complete/full", keybindings_complete_full);
  TEST ("keybindings/complete/has-prefix", keybindings_complete_has_prefix);
  TEST ("keybindings/complete/no-prefix", keybindings_complete_no_prefix);
  TEST ("keybindings/complete/nothing", keybindings_complete_nothing);
  TEST ("keybindings/home/nonsmart", keybindings_home_nonsmart);
  TEST ("keybindings/home/smart", keybindings_home_smart);
  TEST ("keybindings/end/nonsmart", keybindings_end_nonsmart);
  TEST ("keybindings/end/smart", keybindings_end_smart);

  /* Because they can fail if a selection
   * is made at the time of running the test.
   */
  if (g_test_thorough ())
    {
      TEST ("selection/from-input", selection_inside_input);
      TEST ("selection/outside-input", selection_outside_input);
      TEST ("selection/select-all", selection_select_all);
      TEST ("selection/up", selection_up);
      TEST ("selection/down", selection_down);
    }

  TEST ("history/up/empty", history_up_empty);
  TEST ("history/up/nonempty", history_up_nonempty);
  TEST ("history/up/past-top", history_up_past_top);
  TEST ("history/down/empty", history_down_empty);
  TEST ("history/down/nonempty", history_down_nonempty);
  TEST ("history/restore/empty", history_restore_empty);
  TEST ("history/restore/nonempty", history_restore_nonempty);
  TEST ("history/restore/past-end", history_restore_past_end);
  TEST ("history/trailing-whitespace", history_trailing_whitespace);

#undef TEST

  return testing_run_tests ();
}
