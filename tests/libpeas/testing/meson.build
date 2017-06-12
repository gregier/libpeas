libtesting_sources = [
  'testing.c',
  'testing.h',
  'testing-extension.c',
  'testing-extension.h',
]


libtesting_deps = [
  libtesting_util_dep,
  libintrospection_dep,
]

libtesting = library(
  'testing',
  libtesting_sources,

  dependencies: libtesting_deps,
  install: false,
)

libtesting_dep = declare_dependency(sources: libtesting_sources,
  dependencies: libtesting_deps,
  link_with: libtesting,
  include_directories: include_directories('.'))
