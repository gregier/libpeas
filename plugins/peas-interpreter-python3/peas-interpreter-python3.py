# -*- coding: utf-8 -*-
#
# peas-interpreter-python3.py -- A Peas Interpreter for Python 3
#
# Copyright (C) 2006 - Steve Frécinaux
# Copyright (C) 2011 - Garrett Regier
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# Parts from "Interactive Python-GTK Console" (stolen from epiphany's console.py)
#     Copyright (C), 1998 James Henstridge <james@daa.com.au>
#     Copyright (C), 2005 Adam Hooper <adamh@densi.com>
#
# Bits from gedit Python Console Plugin
#     Copyrignt (C), 2005 Raphaël Slinckx

import copy
import os
import rlcompleter
import sys
import traceback

from gi.repository import GObject, Peas


# TODO: implement an IPython mode?
# IPython Console used in Accerciser
#   https://git.gnome.org/browse/accerciser/tree/plugins/ipython_view.py

class PeasInterpreterPython3(GObject.Object, Peas.Interpreter):
    def __init__(self):
        super(PeasInterpreterPython3, self).__init__()

        self.connect("reset", self.on_reset)

        self.__stdout = FakeFile(self.write, sys.stdout.fileno())
        self.__stderr = FakeFile(self.write_error, sys.stderr.fileno())

        self.do_set_namespace(None)

    def do_prompt(self):
        if not self.__in_block and not self.__in_multiline:
            if hasattr(sys, 'ps1'):
                return str(sys.ps1)

        else:
            if hasattr(sys, 'ps2'):
                return (sys.ps2)

        return ''

    def do_execute(self, code):
        success = False

        # We only get passed an exact statement
        # str.join()?
        self.__code += code + '\n'

        if self.__code.rstrip().endswith(':') or \
              (self.__in_block and not self.__code.endswith('\n\n')):
            self.__in_block = True
            return

        elif self.__code.endswith('\\'):
            self.__in_multiline = True
            return

        sys.stdout, self.__stdout = self.__stdout, sys.stdout
        sys.stderr, self.__stderr = self.__stderr, sys.stderr

        try:
            try:
                retval = eval(self.__code,
                              self.__namespace,
                              self.__namespace)

            except SyntaxError:
                exec(self.__code, self.__namespace)

            else:
                # This also sets '_'. Bug #607963
                sys.displayhook(retval)

        except SystemExit:
            self.reset()

        except:
            # TODO: sys.excepthook(*sys.exc_info())

            # Cause only one write-error to be emitted,
            # using traceback.print_exc() would cause multiple.
            self.write_error(traceback.format_exc(0))

        else:
            success = True

        sys.stdout, self.__stdout = self.__stdout, sys.stdout
        sys.stderr, self.__stderr = self.__stderr, sys.stderr

        self.__code = ''
        self.__in_block = False
        self.__in_multiline = False

        return success

    def do_complete(self, code):
        completions = []

        try:
            words = code.split()

            if len(words) == 0:
                word = ''
                text_prefix = code
            else:
                word = words[-1]
                text_prefix = code[:-len(word)]

            for match in self.__completer.global_matches(word):
                # The '(' messes with GTK+'s text wrapping
                if match.endswith('('):
                    match = match[:-1]
                else:
                    match += ' '

                text = text_prefix + match
                completion = Peas.InterpreterCompletion.new(match, text)
                completions.append(completion)

        except:
            pass

        return completions

    def do_get_namespace(self):
        return self.__original_namespace

    def do_set_namespace(self, namespace):
        if namespace is None:
            self.__original_namespace = {}

        else:
            self.__original_namespace = namespace

        self.on_reset()

    # Isn't this automatically setup for up?
    def on_reset(self, unused=None):
        self.__code = ''
        self.__in_block = False
        self.__in_multiline = False

        sys.ps1 = '>>> '
        sys.ps2 = '... '

        self.__namespace = copy.copy(self.__original_namespace)

        if os.getenv('PEAS_DEBUG') is not None:
            if not self.__namespace.has_key('self'):
                self.__namespace['self'] = self

        try:
            self.__completer = rlcompleter.Completer(self.__namespace)

        except:
            pass


class FakeFile:
    """A fake output file object."""

    def __init__(self, callback, fileno):
        self.__callback = callback
        self.__fileno = fileno

    def close(self):
         pass

    def fileno(self):
        return self.__fileno

    def flush(self):
         pass

    def isatty(self):
        return False

    def read(self, size=None):
        raise IOError('File not open for reading')

    def readline(self, size=None):
        raise IOError('File not open for reading')

    def readlines(self, size=None):
        raise IOError('File not open for reading')

    def seek(self, offset, whence=0):
        raise IOError(29, 'Illegal seek')

    def tell(self):
        raise IOError(29, 'Illegal seek')

    def truncate(self, size=None):
        raise IOError(29, 'Illegal seek')

    def write(self, string):
        self.__callback(string)

    def writelines(self, strings):
        self.__callback(''.join(strings))

# ex:ts=4:et:
