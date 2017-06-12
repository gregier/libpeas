libself_dep_sources = [
  'self-dep-plugin.c',
  'self-dep-plugin.h',
]

copy_to_builddir = [
  'self-dep.plugin',
]


libself_dep = shared_library(
  'self-dep',
  libself_dep_sources,

  dependencies: generic_plugin_deps,
  install: false,
)

custom_target('generic-plugins-self-dep',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
