function callable_extension() {
  this.call_with_return = function() {
    return "Hello, World!"
  };
  this.call_no_args = function() {
  };
  this.call_single_arg = function() {
    return true
  };
  this.call_multi_args = function() {
    return [ true, true, true ]
  };
};

function properties_extension() {
  this.read_only = "read-only";
  this.readwrite = "readwrite";
};

extensions = {
  "IntrospectionCallable": callable_extension,
  "IntrospectionProperties" : properties_extension
};
