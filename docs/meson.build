subdir('xml')

src_dirs = [
  join_paths(meson.source_root(), 'libpeas')
]

doc_deps = [
  libpeas_dep
]

ignore_headers = [
  'peas-debug.h',
  'peas-dirs.h',
  'peas-engine-priv.h',
  'peas-gtk-disable-plugins-dialog.h',
  'peas-gtk-plugin-manager-store.h',
  'peas-i18n.h',
  'peas-introspection.h',
  'peas-marshal.h',
  'peas-plugin-info-priv.h',
  'peas-plugin-loader.h',
  'peas-plugin-loader-c.h',
  'peas-utils.h',
]

glib_prefix = libgio_dep.get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')
docpath = join_paths(get_option('datadir'), 'gtk-doc', 'html')

if enable_gtk
  doc_deps += libpeas_gtk_dep
  src_dirs += join_paths(meson.source_root(), 'libpeas-gtk')
endif


gnome.gtkdoc('libpeas',
  main_xml: 'libpeas-docs.sgml',
  src_dir: src_dirs,
  dependencies: doc_deps,
  gobject_typesfile: 'libpeas.types',
  scan_args: [
    '--rebuild-types',
    '--ignore-headers=' + ' '.join(ignore_headers),
  ],
  fixxref_args: [
    '--html-dir=@0@'.format(docpath),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gi')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gtk3')),
  ],
  install_dir: 'libpeas',
  install: true
)
