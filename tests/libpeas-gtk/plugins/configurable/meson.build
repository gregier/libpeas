libconfigurable_sources = [
  'configurable-plugin.c',
  'configurable-plugin.h',
]

copy_to_builddir = [
  'configurable.plugin',
]


libconfigurable = shared_library(
  'configurable',
  libconfigurable_sources,

  dependencies: peas_gtk_plugin_deps,
  install: false,
)


custom_target('libpeas-gtk-plugins-configurable',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
