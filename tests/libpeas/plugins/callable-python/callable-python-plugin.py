# -*- coding: utf-8 -*-
# ex:set ts=4 et sw=4 ai:

import gobject
from gi.repository import Introspection

class CallablePythonPlugin(gobject.GObject, Introspection.Callable):
    __gtype__ = "CallablePythonPlugin"

    def do_call_with_return(self):
        return "Hello, World!";

    def do_call_single_arg(self, called):
        called = True

    def do_call_multi_args(self, called_1, called_2, called_3):
        called_1 = True
        called_2 = True
        called_3 = True
