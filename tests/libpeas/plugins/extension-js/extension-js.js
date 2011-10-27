function callable_extension() {
}

callable_extension.prototype = {
  activate: function() {
  },
  deactivate: function() {
  },
  call_with_return: function() {
    return "Hello, World!"
  },
  call_no_args: function() {
  },
  call_single_arg: function() {
    return true
  },
  call_multi_args: function(in_, inout) {
    return [ inout, in_ ]
  }
}

function properties_extension() {
  this.read_only = "read-only";
  this.readwrite = "readwrite";
}

var extensions = {
  "PeasActivatable": callable_extension,
  "IntrospectionCallable": callable_extension,
  "IntrospectionProperties": properties_extension,
  "IntrospectionHasPrerequisite": callable_extension
}
