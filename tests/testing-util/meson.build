libtesting_util_sources = [
  'testing-util.c',
  'testing-util.h',
]


libtesting_util_deps = [
  libpeas_dep,
]

libtesting_util = library(
  'testing-util',
  libtesting_util_sources,

  dependencies: libtesting_util_deps,
  install: false,
)

libtesting_util_dep = declare_dependency(sources: libtesting_util_sources,
  dependencies: libtesting_util_deps,
  link_with: libtesting_util,
  include_directories: include_directories('.'))
