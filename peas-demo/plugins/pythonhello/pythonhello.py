# -*- coding: utf-8 -*-
# ex:set ts=4 et sw=4 ai:

import gobject
import libpeas
import gtk

LABEL_STRING="Python Says Hello!"

class PythonHelloPlugin(libpeas.Plugin):
    def do_activate(self, window):
        print "PythonHelloPlugin.do_activate", repr(window)
        window._pythonhello_label = gtk.Label(LABEL_STRING)
        window._pythonhello_label.show()
        window.get_child().pack_start(window._pythonhello_label)

    def do_deactivate(self, window):
        print "PythonHelloPlugin.do_deactivate", repr(window)
        window.get_child().remove(window._pythonhello_label)
        window._pythonhello_label.destroy()

gobject.type_register(PythonHelloPlugin)
