libintrospection_sources = [
  'introspection-abstract.c',
  'introspection-abstract.h',
  'introspection-base.c',
  'introspection-base.h',
  'introspection-callable.c',
  'introspection-callable.h',
  'introspection-has-prerequisite.c',
  'introspection-has-prerequisite.h',
  'introspection-prerequisite.c',
  'introspection-prerequisite.h',
  'introspection-unimplemented.c',
  'introspection-unimplemented.h',
]


libintrospection_deps = [
  libpeas_dep,
]

libintrospection = library(
  'introspection-1.0',
  libintrospection_sources,

  dependencies: libintrospection_deps,
  install: false,
)

libintrospection_dep = declare_dependency(sources: libintrospection_sources,
  dependencies: libintrospection_deps,
  link_with: libintrospection,
  include_directories: include_directories('.'))


if get_option('with_introspection')
  libintrospection_gir = gnome.generate_gir(libintrospection,
    sources: libintrospection_sources,
    nsversion: '1.0',
    namespace: 'Introspection',
    symbol_prefix: 'introspection',
    identifier_prefix: 'Introspection',
    link_with: libintrospection,
    includes: [ 'Peas-1.0' ],
    install: false,
    extra_args: [
      '--warn-all',
      '--include-uninstalled=@0@/libpeas/Peas-1.0.gir'.format(meson.build_root()),
    ],
  )

  libintrospection_gir_dep = declare_dependency(sources: libintrospection_gir)
endif
