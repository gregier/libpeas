/*
 * peas-demo-window.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-demo-window.h"

G_DEFINE_TYPE (DemoWindow, demo_window, GTK_TYPE_WINDOW);

static void
demo_window_init (DemoWindow *dw)
{
  DemoWindowClass *klass = DEMO_WINDOW_GET_CLASS (dw);
  gchar *label;

  dw->box = gtk_vbox_new (TRUE, 6);
  gtk_container_add (GTK_CONTAINER (dw), dw->box);

  label = g_strdup_printf ("Peas Window %d", ++(klass->n_windows));
  gtk_window_set_title (GTK_WINDOW (dw), label);
  g_free (label);
}

static void
on_extension_added (PeasExtensionSet *set,
                    PeasPluginInfo   *info,
                    PeasExtension    *exten,
                    DemoWindow       *dw)
{
  peas_activatable_activate (PEAS_ACTIVATABLE (exten));
}

static void
on_extension_removed (PeasExtensionSet *set,
                      PeasPluginInfo   *info,
                      PeasExtension    *exten,
                      DemoWindow       *dw)
{
  peas_activatable_deactivate (PEAS_ACTIVATABLE (exten));
}

static gboolean
on_delete_event (GtkWidget *window,
                 GdkEvent  *event,
                 gpointer   user_data)
{
  DemoWindow *dw = DEMO_WINDOW (window);
  peas_extension_set_call (dw->exten_set, "deactivate");

  return FALSE;
}

static void
demo_window_set_data (DemoWindow *dw,
                      PeasEngine *engine)
{
  dw->engine = engine;
  g_object_ref (dw->engine);

  dw->exten_set = peas_extension_set_new (engine, PEAS_TYPE_ACTIVATABLE,
                                          "object", dw,
                                          NULL);

  peas_extension_set_call (dw->exten_set, "activate");

  g_signal_connect (dw->exten_set, "extension-added", G_CALLBACK (on_extension_added), dw);
  g_signal_connect (dw->exten_set, "extension-removed", G_CALLBACK (on_extension_removed), dw);
  g_signal_connect (dw, "delete-event", G_CALLBACK (on_delete_event), NULL);
}

static void
demo_window_finalize (GObject *object)
{
  DemoWindow *dw = DEMO_WINDOW (object);

  g_object_unref (dw->exten_set);
  g_object_unref (dw->engine);

  G_OBJECT_CLASS (demo_window_parent_class)->finalize (object);
}

static void
demo_window_class_init (DemoWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = demo_window_finalize;

  klass->n_windows = 0;
}

GtkWidget *
demo_window_new (PeasEngine *engine)
{
  DemoWindow *dw;

  dw = DEMO_WINDOW (g_object_new (DEMO_TYPE_WINDOW, NULL));
  demo_window_set_data (dw, engine);

  return GTK_WIDGET (dw);
}
