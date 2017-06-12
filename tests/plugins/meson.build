generic_plugin_deps = [
  libpeas_dep,
]


subdir('builtin')
subdir('has-dep')
subdir('loadable')
subdir('self-dep')


copy_to_builddir = [
  'full-info.plugin',
  'min-info.plugin',
  'unavailable.plugin',
]

custom_target('generic-plugins',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
