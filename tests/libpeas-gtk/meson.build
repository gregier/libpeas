subdir('plugins')
subdir('testing')


test_names = [
  'plugin-manager',
  'plugin-manager-store',
  'plugin-manager-view',
]

test_deps = [
  libpeas_gtk_dep,
  libtesting_gtk_dep,
]


foreach test_name: test_names
  test_exec = executable(test_name,
    '@0@.c'.format(test_name),
    dependencies: test_deps,
    install: false,
  )

  test(test_name, test_exec)
endforeach
