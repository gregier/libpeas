callable_extension = {
  call_with_return: function() {
    return "Hello, World!"
  },
  call_no_args: function() {
  },
  call_single_arg: function() {
    return true
  },
  call_mulit_args: function() {
    return [ true, true, true ]
  }
};

properties_extension = {
  read_only: "read-only",
  readwrite: "readwrite"
};

extensions = {
  "IntrospectionCallable": callable_extension,
  "IntrospectionProperties" : properties_extension
};
