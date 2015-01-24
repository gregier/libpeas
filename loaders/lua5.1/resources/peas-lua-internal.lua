--
--  Copyright (C) 2015 - Garrett Regier
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

local debug = require 'debug'
local package = require 'package'

local lgi = require 'lgi'
local GObject = lgi.GObject
local Peas = lgi.Peas


local Hooks = {}
Hooks.__index = Hooks

function Hooks.new()
    local self = { priv = {} }
    setmetatable(self, Hooks)

    self.priv.module_cache = {}
    self.priv.extension_cache = {}
    return self
end

function Hooks:failed(msg)
    -- This is implemented by the plugin loader
    error('Hooks:failed() was not implemented!')
end

local function add_package_path(package_path)
    local paths = (';%s/?.lua;%s/?/init.lua'):format(package_path,
                                                     package_path)

    if not package.path:find(paths, 1, true) then
        package.path = package.path .. paths
    end
end

local function format_plugin_exception(err)
    -- Format the error even if given a userdata
    local formatted = debug.traceback(tostring(err), 2)

    if type(formatted) ~= 'string' then
        return formatted
    end

    -- Remove all mentions of this file
    local lines = {}
    for line in formatted:gmatch('([^\n]+\n?)') do
        if line:find('peas-lua-internal.lua', 1, true) then
            break
        end

        table.insert(lines, line)
    end

    return table.concat(lines, '')
end

function Hooks:load(filename, module_dir, module_name)
    local module = self.priv.module_cache[filename]

    if module ~= nil then
        return module ~= false
    end

    if package.loaded[module_name] ~= nil then
        local msg = ("Error loading plugin '%s': " ..
                     "module name '%s' has already been used")
        self:failed(msg:format(filename, module_name))
    end

    add_package_path(module_dir)

    local success, result = xpcall(function()
        return require(module_name)
    end, format_plugin_exception)

    if not success then
        local msg = "Error loading plugin '%s':\n%s"
        self:failed(msg:format(module_name, tostring(result)))
    end

    if type(result) ~= 'table' then
        self.priv.module_cache[filename] = false

        local msg = "Error loading plugin '%s': expected table, got: %s (%s)"
        self:failed(msg:format(module_name, type(result), tostring(result)))
    end

    self.priv.module_cache[filename] = result
    self.priv.extension_cache[filename] = {}
    return true
end

function Hooks:find_extension_type(filename, gtype)
    local module_gtypes = self.priv.extension_cache[filename]
    local extension_type = module_gtypes[gtype]

    if extension_type ~= nil then
        if extension_type == false then
            return nil
        end

        return extension_type
    end

    for _, value in pairs(self.priv.module_cache[filename]) do
        local value_gtype = value._gtype

        if value_gtype ~= nil then
            if GObject.type_is_a(value_gtype, gtype) then
                module_gtypes[gtype] = value_gtype
                return value_gtype
            end
        end
    end

    module_gtypes[gtype] = false
    return nil
end

local function check_native(native, wrapped, typename)
    local msg = ('Invalid wrapper for %s: %s'):format(typename,
                                                      tostring(wrapped))

    -- Cannot compare userdata directly!
    assert(wrapped ~= nil, msg)
    assert(tostring(native) == tostring(wrapped._native), msg)
end

function Hooks:setup_extension(exten, info)
    local wrapped_exten = GObject.Object(exten, false)
    check_native(exten, wrapped_exten, 'extension')

    local wrapped_info = Peas.PluginInfo(info, false)
    check_native(info, wrapped_info, 'PeasPluginInfo')

    wrapped_exten.priv.plugin_info = wrapped_info
end

function Hooks:garbage_collect()
    collectgarbage()
end

return Hooks.new()

-- ex:ts=4:et:
