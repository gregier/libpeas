project('libpeas', 'c',
  version: '1.21.0',
  license: 'LGPLv2.1+',
  meson_version: '>= 0.38.1',
  default_options: [ 'warning_level=1', 'buildtype=debugoptimized' ],
)

version_arr = meson.project_version().split('.')
peas_version_major = version_arr[0].to_int()
peas_version_minor = version_arr[1].to_int()
peas_version_micro = version_arr[2].to_int()

apiversion = '@0@.0'.format(peas_version_major)

if peas_version_minor.is_odd()
  peas_interface_age = 0
else
  peas_interface_age = peas_version_micro
endif

# maintaining compatibility with the previous libtool versioning
# current = minor * 100 + micro - interface
# revision = interface
soversion = peas_version_major
current = peas_version_minor * 100 + peas_version_micro - peas_interface_age
revision = peas_interface_age
libversion = '@0@.@1@.@2@'.format(soversion, current, revision)

girdir = join_paths(get_option('datadir'), 'gir-1.0')
typelibdir = join_paths(get_option('libdir'), 'girepository-1.0')

conf = configuration_data()
conf.set_quoted('G_LOG_DOMAIN', 'libpeas')
conf.set_quoted('GETTEXT_PACKAGE', 'libpeas')
conf.set_quoted('DATADIR', get_option('datadir'))
conf.set_quoted('LIBDIR', get_option('libdir'))
conf.set_quoted('BUILDDIR', meson.build_root())
conf.set_quoted('SRCDIR', meson.source_root())
conf.set_quoted('PACKAGE_DATADIR', join_paths(get_option('prefix'), get_option('datadir')))
conf.set_quoted('PACKAGE_LIBDIR', join_paths(get_option('prefix'), get_option('libdir')))
conf.set_quoted('PACKAGE_LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
conf.set('GLIB_VERSION_MIN_REQUIRED', 'GLIB_VERSION_2_36')
conf.set('GLIB_VERSION_MAX_ALLOWED', 'GLIB_VERSION_2_38')
conf.set('GDK_VERSION_MIN_REQUIRED', 'GDK_VERSION_3_0')
conf.set('GDK_VERSION_MAX_ALLOWED', 'GDK_VERSION_3_0')

configure_file(
  output: 'config.h',
  configuration: conf,
)

add_project_arguments([
  '-DHAVE_CONFIG_H',
  '-I' + meson.build_root(),
  '-I' + meson.source_root(),
], language: 'c')

cc = meson.get_compiler('c')

test_c_args = [
  '-Wcast-align',
  '-Wdeclaration-after-statement',
  ['-Werror=format-security', '-Werror=format=2'],
  '-Wformat-nonliteral',
  '-Wformat-security',
  '-Wmissing-include-dirs',
  '-Wnested-externs',
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
  '-Wno-strict-aliasing',
  '-Wno-uninitialized',
  '-Wno-unused-parameter',
  '-Wpointer-arith',
  '-Wredundant-decls',
  '-Wshadow',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wundef',
]
#if get_option('buildtype') != 'plain'
#  test_c_args += '-fstack-protector-strong'
#endif
if get_option('enable_profiling')
  test_c_args += '-pg'
endif

project_c_args = []
foreach arg: test_c_args
  if cc.has_multi_arguments(arg)
    project_c_args += arg
  endif
endforeach
add_project_arguments(
  project_c_args,
  language: 'c'
)

libgio_dep = dependency('gio-2.0', version: '>= 2.38.0')
libgmodule_dep = dependency('gmodule-2.0', version: '>= 2.38.0')
libgobject_introspection_dep = dependency('gobject-introspection-1.0',
                                          version: '>= 1.39.0')

enable_gtk = get_option('enable_gtk')
if enable_gtk != 'false'
  libgtk3_dep = dependency('gtk+-3.0', version: '>= 3.0.0',
                           required: enable_gtk == 'true')

  if not libgtk3_dep.found()
    enable_gtk = 'false'
  else
    enable_gtk = 'true'
  endif
endif

enable_glade_catalog = get_option('enable_glade_catalog')
if enable_glade_catalog != 'false'
  gladeui_dep = dependency('gladeui-2.0',
                           required: enable_glade_catalog == 'true')

  if not gladeui_dep.found()
    enable_glade_catalog = 'false'
  else
    enable_glade_catalog = 'true'

    glade_datadir = gladeui_dep.get_pkgconfig_variable('datadir')
    glade_catalogdir = gladeui_dep.get_pkgconfig_variable('catalogdir')
  endif
endif

enable_lua = get_option('enable_lua5_1')
enable_python2 = get_option('enable_python2')
enable_python3 = get_option('enable_python3')

enable_gtk = enable_gtk == 'true'
enable_glade_catalog = enable_glade_catalog == 'true'
enable_lua = enable_lua == 'true'
enable_python2 = enable_python2 == 'true'
enable_python3 = enable_python3 == 'true'


gnome = import('gnome')
pkg = import('pkgconfig')

subdir('libpeas')
subdir('libpeas-gtk')
#subdir('loaders/lua5.1')
#subdir('loaders/python')
#subdir('loaders/python3')
subdir('peas-demo')
subdir('tests/testing-util')
subdir('tests/libpeas')
subdir('tests/libpeas-gtk')
subdir('tests/plugins')

subdir('data/glade')
subdir('data/icons')
subdir('po')

if get_option('enable_gtk_doc')
  subdir('docs')
endif
