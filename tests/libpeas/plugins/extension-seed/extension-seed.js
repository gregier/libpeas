const Peas = imports.gi.Peas;
const Introspection = imports.gi.Introspection;

function extension_js_plugin() {
  this.read_only = "read-only";
  this.readwrite = "readwrite";
}

extension_js_plugin.prototype = {
  activate: function() {
  },
  deactivate: function() {
  },

  get_plugin_info: function() {
    return this.plugin_info;
  },
  get_settings: function() {
    return this.plugin_info.get_settings(null);
  },

  call_with_return: function() {
    return "Hello, World!";
  },
  call_no_args: function() {
  },
  call_single_arg: function() {
    return true;
  },
  call_multi_args: function(in_, inout) {
    return [ inout, in_ ];
  }
};

function missing_prerequisite_extension() {}

var extensions = {
  "PeasActivatable": extension_js_plugin,
  "IntrospectionBase": extension_js_plugin,
  "IntrospectionCallable": extension_js_plugin,
  "IntrospectionProperties": extension_js_plugin,
  "IntrospectionPropertiesPrerequisite": extension_js_plugin,
  "IntrospectionHasPrerequisite": extension_js_plugin,
  "IntrospectionHasMissingPrerequisite": missing_prerequisite_extension
};
