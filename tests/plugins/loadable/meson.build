libloadable_sources = [
  'loadable-plugin.c',
  'loadable-plugin.h',
]

copy_to_builddir = [
  'loadable.plugin',
]


libloadable = shared_library(
  'loadable',
  libloadable_sources,

  dependencies: generic_plugin_deps,
  install: false,
)

custom_target('generic-plugins-loadable',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
