/*
 * peas-interpreter-completion.h
 * This file is part of libpeas
 *
 * Copyright (C) 2012 - Garrett Regier
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

#ifndef __PEAS_INTERPRETER_COMPLETION_H__
#define __PEAS_INTERPRETER_COMPLETION_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_TYPE_INTERPRETER_COMPLETION            (peas_interpreter_completion_get_type())
#define PEAS_INTERPRETER_COMPLETION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_TYPE_INTERPRETER_COMPLETION, PeasInterpreterCompletion))
#define PEAS_INTERPRETER_COMPLETION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_TYPE_INTERPRETER_COMPLETION, PeasInterpreterCompletionClass))
#define PEAS_IS_INTERPRETER_COMPLETION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_TYPE_INTERPRETER_COMPLETION))
#define PEAS_IS_INTERPRETER_COMPLETION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_INTERPRETER_COMPLETION))
#define PEAS_INTERPRETER_COMPLETION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_TYPE_INTERPRETER_COMPLETION, PeasInterpreterCompletionClass))

typedef struct _PeasInterpreterCompletion         PeasInterpreterCompletion;
typedef struct _PeasInterpreterCompletionClass    PeasInterpreterCompletionClass;

/**
 * PeasInterpreterCompletion:
 *
 * The #PeasInterpreterCompletion structure contains only private
 * data and should only be accessed using the provided API.
 */
struct _PeasInterpreterCompletion {
  GObject parent;
};

/**
 * PeasInterpreterCompletionClass:
 * @parent_class: The parent class.
 *
 * The class structure for #PeasInterpreterCompletion.
 */
struct _PeasInterpreterCompletionClass {
  GObjectClass parent_class;

  /*< private >*/
  gpointer padding[8];
};

/*
 * Public methods
 */
GType  peas_interpreter_completion_get_type        (void) G_GNUC_CONST;

PeasInterpreterCompletion *
             peas_interpreter_completion_new       (const gchar               *label,
                                                    const gchar               *text);

const gchar *peas_interpreter_completion_get_label (PeasInterpreterCompletion *completion);
const gchar *peas_interpreter_completion_get_text  (PeasInterpreterCompletion *completion);

G_END_DECLS

#endif /* __PEAS_INTERPRETER_COMPLETION_H__ */
