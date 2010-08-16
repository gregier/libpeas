# -*- coding: utf-8 -*-
# ex:set ts=4 et sw=4 ai:

import gobject
from gi.repository import Peas
from gi.repository import PeasGtk
from gi.repository import Gtk

LABEL_STRING="Python Says Hello!"

class PythonHelloPlugin(gobject.GObject, Peas.Activatable):
    __gtype_name__ = 'PythonHelloPlugin'

    object = gobject.property(type=gobject.GObject)

    def do_activate(self):
        window = self.object
        print "PythonHelloPlugin.do_activate", repr(window)
        window._pythonhello_label = Gtk.Label()
        window._pythonhello_label.set_text(LABEL_STRING)
        window._pythonhello_label.show()
        window.get_child().pack_start(window._pythonhello_label, True, True, 0)

    def do_deactivate(self):
        window = self.object
        print "PythonHelloPlugin.do_deactivate", repr(window)
        window.get_child().remove(window._pythonhello_label)
        window._pythonhello_label.destroy()

    def do_update_state(self):
        print "PythonHelloPlugin.do_update_state", repr(self.object)

class PythonHelloConfigurable(gobject.GObject, PeasGtk.Configurable):
    __gtype_name__ = 'PythonHelloConfigurable'

    def do_create_configure_widget(self):
        return Gtk.Label.new("Python Hello configure widget")
