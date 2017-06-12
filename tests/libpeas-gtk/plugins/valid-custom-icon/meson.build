copy_to_builddir = [
  'valid-custom-icon.plugin',
  'exists.png',
]


custom_target('libpeas-gtk-plugins-valid-custom-icon',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
