embedded_resources = gnome.compile_resources('embedded-resources',
  'embedded.gresource.xml',
  c_name: 'embedded',
  extra_args: [ '--internal', '--manual-register' ]
)

libembedded_sources = [
  'embedded-plugin.c',
  'embedded-plugin.h',

  embedded_resources,
]

copy_to_builddir = [
  'embedded.plugin',
  'embedded-invalid-loader.plugin',
  'embedded-missing-symbol.plugin',
]


libembedded = library(
  'embedded',
  libembedded_sources,

  dependencies: peas_plugin_deps,
  install: false,
)

libembedded_dep = declare_dependency(sources: libembedded_sources,
  dependencies: peas_plugin_deps,
  link_with: libembedded,
  include_directories: include_directories('.'))

custom_target('libpeas-plugins-embedded',
  input: copy_to_builddir,
  output: copy_to_builddir,
  build_by_default: true,
  command: [ 'cp', '@INPUT@', '@OUTDIR@' ],
)
