# -*- coding: utf-8 -*-
# ex:set ts=4 et sw=4 ai:

from gi.repository import Peas
from gi.repository import Gtk

LABEL_STRING="Python Says Hello!"

class PythonHelloPlugin(Peas.Plugin, Peas.Activatable):
    __gtype_name__ = 'PythonHelloPlugin'

    def do_activate(self, window):
        print "PythonHelloPlugin.do_activate", repr(window)
#        window._pythonhello_label = Gtk.Label()
#        window._pythonhello_label.set_text(LABEL_STRING)
#        window._pythonhello_label.show()
#        window.get_child().pack_start(window._pythonhello_label)

    def do_deactivate(self, window):
        print "PythonHelloPlugin.do_deactivate", repr(window)
#        window.get_child().remove(window._pythonhello_label)
#        window._pythonhello_label.destroy()

    def do_update_state(self, window):
        print "PythonHelloPlugin.do_update_state", repr(window)
