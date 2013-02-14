/*
 * peas-gtk-console-history.c
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

#include "peas-gtk-console-history.h"

struct _PeasGtkConsoleHistory
{
  volatile gint refcount;

  GQueue *queue;
  GList  *position;
};

G_DEFINE_BOXED_TYPE (PeasGtkConsoleHistory, peas_gtk_console_history,
                     peas_gtk_console_history_ref,
                     peas_gtk_console_history_unref)

PeasGtkConsoleHistory *
peas_gtk_console_history_new (void)
{
  PeasGtkConsoleHistory *history;

  history = g_new0 (PeasGtkConsoleHistory, 1);
  history->refcount = 1;
  history->queue = g_queue_new ();
  history->position = NULL;

  return history;
}

PeasGtkConsoleHistory *
peas_gtk_console_history_ref (PeasGtkConsoleHistory *history)
{
  g_return_val_if_fail (history != NULL, NULL);

  g_atomic_int_inc (&history->refcount);

  return history;
}

void
peas_gtk_console_history_unref (PeasGtkConsoleHistory *history)
{
  g_return_if_fail (history != NULL);

  if (!g_atomic_int_dec_and_test (&history->refcount))
    return;

  peas_gtk_console_history_clear (history);
  g_queue_free (history->queue);

  g_free (history);
}

void
peas_gtk_console_history_add (PeasGtkConsoleHistory *history,
                              const gchar           *text)
{
  const gchar *pos;

  g_return_if_fail (history != NULL);
  g_return_if_fail (text != NULL);

  /* Don't add the text to the queue if it is blank */
  for (pos = text; *pos != '\0'; ++pos)
    {
      if (!g_ascii_isspace (*pos))
        {
          g_queue_push_head (history->queue, g_strdup (text));
          break;
        }
    }

  history->position = NULL;
}

void
peas_gtk_console_history_clear (PeasGtkConsoleHistory *history)
{
  g_return_if_fail (history != NULL);

  g_queue_foreach (history->queue, (GFunc) g_free, NULL);

  g_queue_clear (history->queue);
}

void
peas_gtk_console_history_next (PeasGtkConsoleHistory *history)
{
  g_return_if_fail (history != NULL);

  if (history->position == NULL)
    {
      history->position = g_queue_peek_head_link (history->queue);
    }
  else
    {
      GList *position = g_list_next (history->position);

      if (position != NULL)
        history->position = position;
    }
}

void
peas_gtk_console_history_previous (PeasGtkConsoleHistory *history)
{
  g_return_if_fail (history != NULL);

  if (history->position == NULL)
    return;

  history->position = g_list_previous (history->position);
}

const gchar *
peas_gtk_console_history_get (PeasGtkConsoleHistory *history)
{
  g_return_val_if_fail (history != NULL, NULL);

  if (history->position == NULL)
    return NULL;

  return history->position->data;
}
