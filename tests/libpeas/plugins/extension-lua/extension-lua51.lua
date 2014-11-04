local lgi = require 'lgi'

local GObject = lgi.GObject
local Introspection = lgi.Introspection
local Peas = lgi.Peas


local ExtensionLuaPlugin = GObject.Object:derive('ExtensionLuaPlugin', {
    Peas.Activatable,
    Introspection.Base,
    Introspection.Callable,
    Introspection.HasPrerequisite,
    Introspection.PropertiesPrerequisite,
    Introspection.Properties
})

ExtensionLuaPlugin._property.object =
    GObject.ParamSpecObject('object', 'object', 'object',
                            GObject.Object._gtype,
                            { GObject.ParamFlags.READABLE,
                              GObject.ParamFlags.WRITABLE })

ExtensionLuaPlugin._property.construct_only =
    GObject.ParamSpecString('construct-only', 'construct-only',
                            'construct-only',
                            '',
                            { GObject.ParamFlags.CONSTRUCT_ONLY,
                              GObject.ParamFlags.READABLE,
                              GObject.ParamFlags.WRITABLE })

ExtensionLuaPlugin._property.read_only =
    GObject.ParamSpecString('read-only', 'read-only', 'read-only',
                            '',
                            { GObject.ParamFlags.READABLE })

ExtensionLuaPlugin._property.write_only =
    GObject.ParamSpecString('write-only', 'write-only', 'write-only',
                            '',
                            { GObject.ParamFlags.WRITABLE })

ExtensionLuaPlugin._property.readwrite =
    GObject.ParamSpecString('readwrite', 'readwrite', 'readwrite',
                            '',
                            { GObject.ParamFlags.READABLE,
                              GObject.ParamFlags.WRITABLE })

ExtensionLuaPlugin._property.prerequisite =
    GObject.ParamSpecString('prerequisite', 'prerequisite', 'prerequisite',
                            '',
                            { GObject.ParamFlags.CONSTRUCT_ONLY,
                              GObject.ParamFlags.READABLE,
                              GObject.ParamFlags.WRITABLE })

function ExtensionLuaPlugin:do_activate()
    collectgarbage('restart')
end

function ExtensionLuaPlugin:do_deactivate()
    collectgarbage('stop')
end

function ExtensionLuaPlugin:do_update_state()
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

ExtensionLuaPlugin()
return { ExtensionLuaPlugin }

-- ex:set ts=4 et sw=4 ai:
