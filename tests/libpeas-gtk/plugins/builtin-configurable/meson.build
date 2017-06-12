libbuiltin_configurable_sources = [
  'builtin-configurable-plugin.c',
  'builtin-configurable-plugin.h',
]

copy_to_builddir = [
  'builtin-configurable.plugin',
]


libbuiltin_configurable = shared_library(
  'builtin-configurable',
  libbuiltin_configurable_sources,

  dependencies: peas_gtk_plugin_deps,
  install: false,
)

custom_target('libpeas-gtk-plugins-builtin-configurable',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
