/*
 * minizip-priv.h
 * This file is part of libminizip-glib
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

#ifndef __MINIZIP_PRIV_H__
#define __MINIZIP_PRIV_H__

#include <gio/gio.h>

#include "minizip/ioapi.h"

#define ZIP_MAX_PATH_LENGTH 257
#define BUFFER_SIZE 8192

/*
static zlib_filefunc_def filefuncs = {
 ...
};

zlib_filefunc_def * _minizip_io_get_filefuncs (GInputStream  *input_stream,
                                               GOutputStream *output_stream);
*/

void _minizip_io_fill_filefuncs (zlib_filefunc_def *filefuncs,
                                 GInputStream      *input_stream,
                                 GOutputStream     *output_stream);

G_END_DECLS

#endif /* __MINIZIP_PRIV_H__ */
