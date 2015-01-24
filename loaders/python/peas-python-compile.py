# -*- coding: utf-8 -*-

#  Copyright (C) 2015 - Garrett Regier
#
# libpeas is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# libpeas is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

import marshal
import sys
import traceback


def compile_file(filename, output):
    """Byte-compiles a Python source file to Python bytecode.

       Unlike py_compile the output is not prefixed
       by a magic value, mtime or size.
    """
    # Open with universal newlines
    with open(filename, 'U') as f:
        code = f.read() + '\n'

    try:
        code_object = compile(code, filename, 'exec')

    except (SyntaxError, TypeError) as error:
        tb = traceback.format_exc(0).rstrip('\n')
        raise Exception('Failed to compile "{0}":\n{1}'.format(filename, tb))

    with open(output, 'wb') as f:
        marshal.dump(code_object, f)
        f.flush()


def main(args):
    try:
        for i in range(0, len(args), 2):
            compile_file(args[i], args[i + 1])

    except Exception as error:
        sys.exit('Error: {0!s}'.format(error))


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

# ex:ts=4:et:
