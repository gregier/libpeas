libpeas_gtk_public_headers = [
  'peas-gtk.h',
  'peas-gtk-autocleanups.h',
  'peas-gtk-configurable.h',
  'peas-gtk-plugin-manager.h',
  'peas-gtk-plugin-manager-view.h',
]

libpeas_gtk_private_headers = [
  'peas-gtk-disable-plugins-dialog.h',
  'peas-gtk-plugin-manager-store.h',
]

libpeas_gtk_sources = [
  'peas-gtk-configurable.c',
  'peas-gtk-disable-plugins-dialog.c',
  'peas-gtk-plugin-manager.c',
  'peas-gtk-plugin-manager-store.c',
  'peas-gtk-plugin-manager-view.c',
]


libpeas_gtk_deps = [
  libpeas_dep,
  libgtk3_dep,
]

libpeas_gtk = library(
  'peas-gtk-' + apiversion,
  libpeas_gtk_sources,

  link_depends: 'peas-gtk.map',
  c_args: [],
  link_args: [ '-Wl,--version-script,' + join_paths(meson.current_source_dir(), 'peas-gtk.map') ],
  dependencies: libpeas_gtk_deps,
  soversion: soversion,
  version: libversion,
  install: true,
  install_dir: get_option('libdir'),
)

libpeas_gtk_dep = declare_dependency(sources: libpeas_gtk_sources,
  dependencies: libpeas_gtk_deps,
  link_with: libpeas_gtk)

#AM_CPPFLAGS += -xobjective-c
#libpeas_gtk_1_0_la_LDFLAGS += -framework Cocoa


if get_option('with_introspection')
  libpeas_gtk_gir = gnome.generate_gir(libpeas_gtk,
    sources: libpeas_gtk_public_headers + libpeas_gtk_sources,
    nsversion: apiversion,
    namespace: 'PeasGtk',
    symbol_prefix: 'peas_gtk',
    identifier_prefix: 'PeasGtk',
    link_with: libpeas_gtk,
    includes: [ 'Peas-1.0', 'Gtk-3.0'],
    install: true,
    install_dir_gir: girdir,
    install_dir_typelib: typelibdir,
    extra_args: [
      '--warn-all',
      '--c-include=peas-gtk.h',
      '--include-uninstalled=@0@/libpeas/Peas-1.0.gir'.format(meson.build_root()),
    ],
  )
endif


pkg.generate(
  name: 'libpeas-gtk',
  libraries: [libpeas_gtk],
  version: meson.project_version(),
  subdirs: 'libpeas-gtk-@0@'.format(apiversion),
  filebase: 'libpeas-gtk-@0@'.format(apiversion),
  description: 'GTK+-3.0 widgets for libpeas',
  requires: [ 'libpeas-@0@'.format(apiversion), 'gtk+-3.0' ],
)
