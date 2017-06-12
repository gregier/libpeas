icons_dir = join_paths(get_option('datadir'), 'icons/hicolor')

icon_dirs = [
  '16x16',
  '22x22',
  '32x32',
  'scalable',
]


foreach icon_dir: icon_dirs
  install_subdir(icon_dir,
    install_dir: icons_dir,
  )
endforeach
