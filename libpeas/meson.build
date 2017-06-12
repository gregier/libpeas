libpeas_marshalers = gnome.genmarshal('peas-marshal',
  sources: 'peas-marshal.list',
  prefix: 'peas_cclosure_marshal',
  valist_marshallers: true)


libpeas_public_headers = [
  'peas.h',
  'peas-activatable.h',
  'peas-autocleanups.h',
  'peas-engine.h',
  'peas-extension-base.h',
  'peas-extension.h',
  'peas-extension-set.h',
  'peas-object-module.h',
  'peas-plugin-info.h',
]

libpeas_private_headers = [
  'peas-debug.h',
  'peas-dirs.h',
  'peas-engine-priv.h',
  'peas-i18n.h',
  'peas-introspection.h',
  'peas-plugin-info-priv.h',
  'peas-plugin-loader.h',
  'peas-plugin-loader-c.h',
  'peas-utils.h',
]

libpeas_sources = [
  'peas-activatable.c',
  'peas-debug.c',
  'peas-dirs.c',
  'peas-engine.c',
  'peas-extension-base.c',
  'peas-extension.c',
  'peas-extension-set.c',
  'peas-i18n.c',
  'peas-introspection.c',
  'peas-object-module.c',
  'peas-plugin-info.c',
  'peas-plugin-loader.c',
  'peas-plugin-loader-c.c',
  'peas-utils.c',
]
libpeas_sources += libpeas_marshalers


libpeas_deps = [
  libgio_dep,
  libgmodule_dep,
  libgobject_introspection_dep
]

libpeas = library(
  'peas-' + apiversion,
  libpeas_sources,

  link_depends: 'peas.map',
  link_args: [ '-Wl,--version-script,' + join_paths(meson.current_source_dir(), 'peas.map') ],
  dependencies: libpeas_deps,
  soversion: soversion,
  version: libversion,
  install: true,
)

libpeas_dep = declare_dependency(sources: libpeas_sources,
  dependencies: libpeas_deps,
  link_with: libpeas)

#AM_CPPFLAGS += -xobjective-c
#libpeas_1_0_la_LDFLAGS += -framework Cocoa


if get_option('with_introspection')
  libpeas_gir = gnome.generate_gir(libpeas,
    sources: libpeas_public_headers + libpeas_sources,
    nsversion: apiversion,
    namespace: 'Peas',
    symbol_prefix: 'peas',
    identifier_prefix: 'Peas',
    link_with: libpeas,
    includes: [ 'GModule-2.0', 'Gio-2.0', 'GIRepository-2.0' ],
    install: true,
    install_dir_gir: girdir,
    install_dir_typelib: typelibdir,
    extra_args: [ '--c-include=peas.h', '--warn-all' ],
  )
endif


pkg.generate(
  name: 'libpeas',
  libraries: [libpeas],
  version: meson.project_version(),
  subdirs: 'libpeas-@0@'.format(apiversion),
  filebase: 'libpeas-@0@'.format(apiversion),
  description: 'A GObject plugins library',
  requires: [ 'gmodule-2.0', 'gio-2.0' ],
)
