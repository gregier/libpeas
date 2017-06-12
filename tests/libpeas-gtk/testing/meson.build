libtesting_gtk_sources = [
  'testing.c',
  'testing.h',
]


libtesting_gtk_deps = [
  libpeas_gtk_dep,
  libtesting_util_dep,
]

libtesting_gtk = library(
  'testing-gtk',
  libtesting_gtk_sources,

  dependencies: libtesting_gtk_deps,
  install: false,
)

libtesting_gtk_dep = declare_dependency(sources: libtesting_gtk_sources,
  dependencies: libtesting_gtk_deps,
  link_with: libtesting_gtk,
  include_directories: include_directories('.'))
