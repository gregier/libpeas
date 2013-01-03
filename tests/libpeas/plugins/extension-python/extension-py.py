# -*- coding: utf-8 -*-
# ex:set ts=4 et sw=4 ai:

from gi.repository import GObject, Introspection, Peas

class ExtensionPythonPlugin(GObject.Object, Peas.Activatable,
                            Introspection.Base, Introspection.Callable,
                            Introspection.PropertiesPrerequisite,
                            Introspection.Properties,
                            Introspection.HasPrerequisite):

    object = GObject.property(type=GObject.Object)

    construct_only = GObject.property(type=str)
    read_only = GObject.property(type=str, default="read-only")
    write_only = GObject.property(type=str)
    readwrite = GObject.property(type=str, default="readwrite")
    prerequisite = GObject.property(type=str)

    def do_activate(self):
        pass

    def do_deactivate(self):
        pass

    def do_update_state(self):
        pass

    def do_get_plugin_info(self):
        return self.plugin_info

    def do_get_settings(self):
        return self.plugin_info.get_settings(None)

    def do_call_with_return(self):
        return "Hello, World!"

    def do_call_no_args(self):
        pass

    def do_call_single_arg(self):
        return True

    def do_call_multi_args(self, in_, inout):
        return (inout, in_)
