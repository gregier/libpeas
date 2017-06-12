catalog_conf = configuration_data()
catalog_conf.set('PACKAGE_API_VERSION', apiversion)

configure_file(
  input: 'libpeas-gtk.xml.in',
  output: 'libpeas-gtk.xml',
  configuration: catalog_conf,
)
