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

#include <string.h>


static gint peas_debug_state = -1;


gboolean
peas_debug_enabled (void)
{
  g_assert (peas_debug_state == 0 || peas_debug_state == 1);

  return peas_debug_state == 1;
}

void
peas_debug_init (void)
{
  const gchar *domains;

  domains = g_getenv ("G_MESSAGES_DEBUG");

  if (domains != NULL &&
      (strcmp (domains, "all") == 0 ||
       strstr (domains, G_LOG_DOMAIN) != NULL))
    {
      peas_debug_state = 1;
      return;
    }

  peas_debug_state = g_getenv ("PEAS_DEBUG") == NULL ? 0 : 1;
  if (peas_debug_state == 0)
    return;

  if (domains == NULL)
    {
      g_setenv ("G_MESSAGES_DEBUG", G_LOG_DOMAIN, TRUE);
    }
  else
    {
      gchar *new_domains;

      new_domains = g_strconcat (domains, " ", G_LOG_DOMAIN, NULL);
      g_setenv ("G_MESSAGES_DEBUG", new_domains, TRUE);

      g_free (new_domains);
    }
}
