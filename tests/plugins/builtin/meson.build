libbuiltin_sources = [
  'builtin-plugin.c',
  'builtin-plugin.h',
]

copy_to_builddir = [
  'builtin.plugin',
]


libbuiltin = shared_library(
  'builtin',
  libbuiltin_sources,

  dependencies: generic_plugin_deps,
  install: false,
)

custom_target('generic-plugins-builtin',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
