/*
 * peas-debug.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Garrett Regier
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-debug.h"


static void
debug_log_handler (const gchar    *log_domain,
                   GLogLevelFlags  log_level,
                   const gchar    *message,
                   gpointer        user_data)
{
}

void
peas_debug_init (void)
{
  if (g_getenv ("PEAS_DEBUG") == NULL)
    {
      g_log_set_handler (G_LOG_DOMAIN,
                         G_LOG_LEVEL_DEBUG,
                         debug_log_handler,
                         NULL);
    }
  else
    {
      const gchar *g_messages_debug;

      g_messages_debug = g_getenv ("G_MESSAGES_DEBUG");

      if (g_messages_debug == NULL)
        {
          g_setenv ("G_MESSAGES_DEBUG", G_LOG_DOMAIN, TRUE);
        }
      else
        {
          gchar *new_g_messages_debug;

          new_g_messages_debug = g_strconcat (g_messages_debug, " ",
                                              G_LOG_DOMAIN, NULL);
          g_setenv ("G_MESSAGES_DEBUG", new_g_messages_debug, TRUE);

          g_free (new_g_messages_debug);
        }
    }
}
