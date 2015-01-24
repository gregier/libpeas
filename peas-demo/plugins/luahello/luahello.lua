--
--  Copyright (C) 2014 - Garrett Regier
--
-- libpeas is free software; you can redistribute it and/or
-- modify it under the terms of the GNU Lesser General Public
-- License as published by the Free Software Foundation; either
-- version 2.1 of the License, or (at your option) any later version.
--
-- libpeas is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public
-- License along with this library; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

local lgi = require 'lgi'

local GObject = lgi.GObject
local Gtk = lgi.Gtk
local Peas = lgi.Peas
local PeasGtk = lgi.PeasGtk


local LuaHelloPlugin = GObject.Object:derive('LuaHelloPlugin', {
    Peas.Activatable
})

LuaHelloPlugin._property.object =
    GObject.ParamSpecObject('object', 'object', 'object',
                            GObject.Object._gtype,
                            { GObject.ParamFlags.READABLE,
                              GObject.ParamFlags.WRITABLE })

function LuaHelloPlugin:do_activate()
    local window = self.priv.object
    print('LuaHelloPlugin:do_activate', tostring(window))
    self.priv.label = Gtk.Label.new('Lua Says Hello!')
    self.priv.label:show()
    window:get_child():pack_start(self.priv.label, true, true, 0)
end

function LuaHelloPlugin:do_deactivate()
    local window = self.priv.object
    print('LuaHelloPlugin:do_deactivate', tostring(window))
    window:get_child():remove(self.priv.label)
    self.priv.label:destroy()
end

function LuaHelloPlugin:do_update_state()
    local window = self.priv.object
    print('LuaHelloPlugin:do_update_state', tostring(window))
end


local LuaHelloConfigurable = GObject.Object:derive('LuaHelloConfigurable', {
    PeasGtk.Configurable
})

function LuaHelloConfigurable:do_create_configure_widget()
    return Gtk.Label.new('Lua Hello configure widget')
end

return { LuaHelloPlugin, LuaHelloConfigurable }

-- ex:set ts=4 et sw=4 ai:
