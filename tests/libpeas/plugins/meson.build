peas_plugin_deps = [
  libpeas_dep,
  libintrospection_dep,
]


subdir('embedded')
subdir('extension-c')
#subdir('extension-lua')
#subdir('extension-python')


copy_to_builddir = [
  'extension-c-nonexistent.plugin',
  'extension-lua51-nonexistent.plugin',
  'extension-python-nonexistent.plugin',
  'extension-python3-nonexistent.plugin',
  'info-missing-module.plugin',
  'info-missing-name.plugin',
  'invalid.plugin',
  'nonexistent-dep.plugin',
  'not-loadable.plugin',
  'os-dependant-help.plugin',
  'two-deps.plugin',
  'unkown-loader.plugin',
]

custom_target('libpeas-plugins',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
