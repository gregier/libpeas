/*
 * testing-interpreter.h
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

#ifndef __TESTING_INTERPRETER_H__
#define __TESTING_INTERPRETER_H__

#include <glib-object.h>

#include <libpeas/peas-interpreter.h>

G_BEGIN_DECLS

#define TESTING_TYPE_INTERPRETER         (testing_interpreter_get_type ())
#define TESTING_INTERPRETER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_INTERPRETER, TestingInterpreter))
#define TESTING_INTERPRETER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_INTERPRETER, TestingInterpreter))
#define TESTING_IS_INTERPRETER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_INTERPRETER))
#define TESTING_IS_INTERPRETER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_INTERPRETER))
#define TESTING_INTERPRETER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_INTERPRETER, TestingInterpreterClass))

typedef struct _TestingInterpreter        TestingInterpreter;
typedef struct _TestingInterpreterClass   TestingInterpreterClass;

struct _TestingInterpreter {
  GObject parent;
};

struct _TestingInterpreterClass {
  GObjectClass parent_class;
};

GType            testing_interpreter_get_type  (void) G_GNUC_CONST;

PeasInterpreter *testing_interpreter_new       (void);

const gchar     *testing_interpreter_get_code  (TestingInterpreter *interpreter);

G_END_DECLS

#endif /* __TESTING_INTERPRETER_H__ */
