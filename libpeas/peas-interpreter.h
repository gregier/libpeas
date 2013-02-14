/*
 * peas-interpreter.h
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

#ifndef __PEAS_INTERPRETER_H__
#define __PEAS_INTERPRETER_H__

#include <glib-object.h>

#include "peas-interpreter-completion.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_TYPE_INTERPRETER             (peas_interpreter_get_type ())
#define PEAS_INTERPRETER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_INTERPRETER, PeasInterpreter))
#define PEAS_INTERPRETER_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), PEAS_TYPE_INTERPRETER, PeasInterpreterInterface))
#define PEAS_IS_INTERPRETER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_INTERPRETER))
#define PEAS_INTERPRETER_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PEAS_TYPE_INTERPRETER, PeasInterpreterInterface))

/**
 * PeasInterpreter:
 *
 * The #PeasInterpreter structure contains only private data and should only
 * be accessed using the provided API.
 */
typedef struct _PeasInterpreter           PeasInterpreter; /* dummy typedef */
typedef struct _PeasInterpreterInterface  PeasInterpreterInterface;

/**
 * PeasInterpreterInterface:
 * @g_iface: The parent interface.
 * @complete: Completes the given code.
 * @execute: Executes the given code.
 * @prompt: Get the prompt.
 * @get_namespace: Gets the namespace.
 * @set_namespace: Sets the namespace.
 * @reset: Signal class handler for the #PeasInterpreter::reset signal.
 * @write: Signal class handler for the #PeasInterpreter::write signal.
 * @write_error: Signal class handler for the
 *               #PeasInterpreter::write_error signal.
 *
 * Provides and iterface for interpreters.
 */
struct _PeasInterpreterInterface {
  GTypeInterface g_iface;

  /* Virtual methods */
  GList             *(*complete)        (PeasInterpreter  *interpreter,
                                         const gchar      *code);
  gboolean           (*execute)         (PeasInterpreter  *interpreter,
                                         const gchar      *code);
  gchar             *(*prompt)          (PeasInterpreter  *interpreter);

  const GHashTable  *(*get_namespace)   (PeasInterpreter  *interpreter);
  void               (*set_namespace)   (PeasInterpreter  *interpreter,
                                         const GHashTable *namespace_);

  /* Signals */
  void    (*reset)                      (PeasInterpreter  *interpreter);
  void    (*write)                      (PeasInterpreter  *interpreter,
                                         const gchar      *text);
  void    (*write_error)                (PeasInterpreter  *interpreter,
                                         const gchar      *text);

  /*< private >*/
  gpointer padding[16];
};

/* Maybe we should loosen the restrictions here as many interpreters
 * do very different things, and emulating what is know is nice.
 *
 * Examples:
 *      complete() -> all_matches, string_to_change input to
 *      remove write_error() -> write already writes the output
 *      some do histroy a specific way
 *
 *      allow returning a GObject for windowing specific interpreters
 *      (i.e. allow us to how a VTE terminal for libpeas-gtk users)
 *
 *
 * Maybe only PeasGtkConsole should be public and the rest of the API
 * be private libpeas only?
 *  peas_gtk_console_new ("peas-interpreter-python3") -> loads it from private
 */

/*
 * Public methods
 */
GType       peas_interpreter_get_type      (void)  G_GNUC_CONST;

GList      *peas_interpreter_complete      (PeasInterpreter  *interpreter,
                                            const gchar      *code);
gboolean    peas_interpreter_execute       (PeasInterpreter  *interpreter,
                                            const gchar      *code);
gchar      *peas_interpreter_prompt        (PeasInterpreter  *interpreter);

void        peas_interpreter_reset         (PeasInterpreter  *interpreter);
void        peas_interpreter_write         (PeasInterpreter  *interpreter,
                                            const gchar      *text);
void        peas_interpreter_write_error   (PeasInterpreter  *interpreter,
                                            const gchar      *text);

const GHashTable *
            peas_interpreter_get_namespace (PeasInterpreter  *interpreter);
void        peas_interpreter_set_namespace (PeasInterpreter  *interpreter,
                                            const GHashTable *namespace_);

G_END_DECLS

#endif /* __PEAS_INTERPRETER_H__ */
