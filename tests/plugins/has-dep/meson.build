libhas_dep_sources = [
  'has-dep-plugin.c',
  'has-dep-plugin.h',
]

copy_to_builddir = [
  'has-dep.plugin',
]


libhas_dep = shared_library(
  'has-dep',
  libhas_dep_sources,

  dependencies: generic_plugin_deps,
  install: false,
)

custom_target('generic-plugins-has-dep',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
