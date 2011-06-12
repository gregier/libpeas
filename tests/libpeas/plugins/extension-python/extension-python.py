# -*- coding: utf-8 -*-
# ex:set ts=4 et sw=4 ai:

import gobject
from gi.repository import Introspection, Peas

class CallablePythonPlugin(gobject.GObject, Introspection.Callable):
    __gtype_name__ = "CallablePythonPlugin"

    def do_call_with_return(self):
        return "Hello, World!";

    def do_call_no_args(self):
        pass

    def do_call_single_arg(self):
        return True

    def do_call_multi_args(self, in_, inout):
        return (inout, in_)

class PropertiesPythonPlugin(gobject.GObject, Introspection.Properties):
    __gtype_name__ = "PropertiesPythonPlugin"

    construct_only = gobject.property(type=str)

    read_only = gobject.property(type=str, default="read-only")
                                      
    write_only = gobject.property(type=str)

    readwrite = gobject.property(type=str, default="readwrite")

class ActivatablePythonExtension(gobject.GObject, Peas.Activatable):
    __gtype_name__ = "ActivatablePythonExtension"

    object = gobject.property(type=gobject.GObject)

    def do_activate(self):
        pass

    def do_deactivate(self):
        pass

    def update_state(self):
        pass
