subdir('introspection')
subdir('plugins')
subdir('testing')


test_names = [
  'engine',
  'extension-c',
  'extension-set',
  'plugin-info',
]

test_deps = [
  libpeas_dep,
  libtesting_dep,
  libembedded_dep,
  libintrospection_gir_dep,
]


foreach test_name: test_names
  test_exec = executable(test_name,
    '@0@.c'.format(test_name),
    dependencies: test_deps,
    install: false,
  )

  test(test_name, test_exec)
endforeach

if enable_lua
  extension_lua = executable('extension-lua',
    'extension-lua.c',
    dependencies: test_deps + [lua_dep],
    install: false,
  )

  test('extension-lua', extension_lua)
endif

if enable_python2
  extension_python2 = executable('extension-python',
    'extension-py.c',
    dependencies: test_deps + [python2_dep],
    install: false,
  )

  test('extension-python2', extension_python2)
endif

if enable_python3
  extension_python3 = executable('extension-python3',
    'extension-py.c',
    dependencies: test_deps + [python3_dep],
    install: false,
  )

  test('extension-python3', extension_python3)
endif
