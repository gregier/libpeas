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
local Introspection = lgi.Introspection
local Peas = lgi.Peas


local ExtensionLuaPlugin = GObject.Object:derive('ExtensionLuaPlugin', {
    Peas.Activatable,
    Introspection.Base,
    Introspection.Callable,
    Introspection.HasPrerequisite
})

ExtensionLuaPlugin._property.object =
    GObject.ParamSpecObject('object', 'object', 'object',
                            GObject.Object._gtype,
                            { GObject.ParamFlags.READABLE,
                              GObject.ParamFlags.WRITABLE })

ExtensionLuaPlugin._property.update_count =
    GObject.ParamSpecInt('update-count', 'update-count', 'update-count',
                         0, 1000000, 0,
                         { GObject.ParamFlags.READABLE })

function ExtensionLuaPlugin:_init()
    self.priv.update_count = 0
end

function ExtensionLuaPlugin:do_activate()
    collectgarbage('restart')
end

function ExtensionLuaPlugin:do_deactivate()
    collectgarbage('stop')
end

function ExtensionLuaPlugin:do_update_state()
    self.priv.update_count = self.priv.update_count + 1
end

function ExtensionLuaPlugin:do_get_plugin_info()
    return self.priv.plugin_info
end

function ExtensionLuaPlugin:do_get_settings()
    return self.priv.plugin_info:get_settings(nil)
end

function ExtensionLuaPlugin:do_call_no_args()
end

function ExtensionLuaPlugin:do_call_with_return()
    return 'Hello, World!'
end

function ExtensionLuaPlugin:do_call_single_arg()
    return true
end

function ExtensionLuaPlugin:do_call_multi_args(in_, inout)
    return inout, in_
end

-- Test strict mode
local UNIQUE = {}

local function assert_error(success, result)
    assert(not success, result)
end

assert_error(pcall(function() _G[UNIQUE] = true end))
assert(pcall(function()
    rawset(_G, UNIQUE, true)
    assert(_G[UNIQUE] == true)
    _G[UNIQUE] = nil
end))
assert_error(pcall(function() _G[UNIQUE] = true end))
assert(pcall(function()
    __STRICT = false
    _G[UNIQUE] = true
    _G[UNIQUE] = nil
    __STRICT = true
end))

return { ExtensionLuaPlugin }

-- ex:set ts=4 et sw=4 ai:
