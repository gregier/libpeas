local lgi = require 'lgi'

local GObject = lgi.GObject
local Gtk = lgi.Gtk
local Peas = lgi.Peas
local PeasGtk = lgi.PeasGtk


LuaHelloPlugin = GObject.Object:derive('LuaHelloPlugin', {
    Peas.Activatable
})

LuaHelloPlugin._property.object =
    GObject.ParamSpecObject('object', 'object', 'object',
                            GObject.Object._gtype,
                            { GObject.ParamFlags.READABLE,
                              GObject.ParamFlags.WRITABLE })

function LuaHelloPlugin:do_activate()
    window = self.priv.object
    print('LuaHelloPlugin:do_activate', tostring(window))
    self.priv.label = Gtk.Label.new('Lua Says Hello!')
    self.priv.label:show()
    window:get_child():pack_start(self.priv.label, true, true, 0)
end

function LuaHelloPlugin:do_deactivate()
    window = self.priv.object
    print('LuaHelloPlugin:do_deactivate', tostring(window))
    window:get_child():remove(self.priv.label)
    self.priv.label:destroy()
end

function LuaHelloPlugin:do_update_state()
    window = self.priv.object
    print('LuaHelloPlugin:do_update_state', tostring(window))
end


LuaHelloConfigurable = GObject.Object:derive('LuaHelloConfigurable', {
    PeasGtk.Configurable
})

function LuaHelloConfigurable:do_create_configure_widget()
    return Gtk.Label.new('Lua Hello configure widget')
end

return { LuaHelloPlugin, LuaHelloConfigurable }

-- ex:set ts=4 et sw=4 ai:
