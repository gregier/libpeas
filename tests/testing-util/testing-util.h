/*
 * testing-util.h
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __TESTING_UTIL_H__
#define __TESTING_UTIL_H__

#include <libpeas/peas-engine.h>

G_BEGIN_DECLS

void        testing_util_init        (void);

PeasEngine *testing_util_engine_new  (void);
void        testing_util_engine_free (PeasEngine *engine);

int         testing_util_run_tests   (void);

G_END_DECLS

#endif /* __TESTING_UTIL_H__ */
