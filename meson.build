project(
  'chill',
  'c',
  version: '1.0.0',
  default_options: ['warning_level=3', 'werror=true']
)

incdir = include_directories('include')

sources = files(
  'src/main.c'
)

executable(
  'chill',
  sources,
  include_directories: incdir,
)