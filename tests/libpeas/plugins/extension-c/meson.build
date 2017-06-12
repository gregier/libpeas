libextension_c_sources = [
  'extension-c-abstract.c',
  'extension-c-abstract.h',
  'extension-c-plugin.c',
  'extension-c-plugin.h',
]

libextension_c_missing_symbol_sources = [
  'extension-c-missing-symbol-plugin.c',
]

copy_to_builddir = [
  'extension-c.gschema.xml',
  'extension-c.plugin',
  'extension-c-missing-symbol.plugin',
]


libextension_c = shared_library(
  'extension-c',
  libextension_c_sources,

  dependencies: peas_plugin_deps,
  install: false,
)

libextension_c_missing_symbol = shared_library(
  'extension-c-missing-symbol',
  libextension_c_missing_symbol_sources,

  dependencies: peas_plugin_deps,
  install: false,
)

custom_target('libpeas-plugins-extension-c',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
