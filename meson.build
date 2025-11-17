project(
  'chill',
  'c',
  version: '1.0.0',
  default_options: ['warning_level=3', 'werror=true', 'c_std=c89']
)

incdir = include_directories('include')

sources = files(
  'src/prompt.c',
  'src/lex.c',
  'src/parser.c',
  'src/env.c',
  'src/tree.c',
  'src/builtin.c',
  'src/cmd.c',
  'src/main.c'
)

## add_project_arguments('-DDEBUG', language: 'c')

executable(
  'chill',
  sources,
  include_directories: incdir,
)