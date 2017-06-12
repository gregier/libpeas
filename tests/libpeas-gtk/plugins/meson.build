peas_gtk_plugin_deps = [
  libpeas_dep,
  libpeas_gtk_dep,
]


subdir('builtin-configurable')
subdir('configurable')
subdir('valid-custom-icon')


copy_to_builddir = [
  'hidden.plugin',
  'invalid-custom-icon.plugin',
  'invalid-stock-icon.plugin',
  'valid-stock-icon.plugin',
]

custom_target('libpeas-gtk-plugins',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
