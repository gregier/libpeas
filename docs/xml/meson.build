ent_conf = configuration_data()
ent_conf.set('PACKAGE', 'libpeas')
ent_conf.set('PACKAGE_BUGREPORT', 'https://bugzilla.gnome.org/enter_bug.cgi?product=libpeas')
ent_conf.set('PACKAGE_NAME', 'libpeas')
ent_conf.set('PACKAGE_STRING', 'libpeas')
ent_conf.set('PACKAGE_TARNAME', 'libpeas-' + meson.project_version())
ent_conf.set('PACKAGE_URL', 'http://wiki.gnome.org/Projets/Libpeas')
ent_conf.set('PACKAGE_VERSION', meson.project_version())
ent_conf.set('PACKAGE_API_VERSION', apiversion)

configure_file(
  input: 'gtkdocentities.ent.in',
  output: 'gtkdocentities.ent',
  configuration: ent_conf,
)
