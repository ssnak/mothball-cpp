project('sim', 'cpp',
  version : '1.0.0',
  default_options : {
    'cpp_std': 'c++20',
    'warning_level': '3',
    'buildtype': 'release'
  }
)


re2c = find_program('re2c')
lexer_cpp = custom_target(
    'lexer_generated.cpp',
    input: 'lexer.re',
    output: 'lexer.cpp',
    command: [re2c, '-o', '@OUTPUT@', '@INPUT@'],
    depend_files: 'lexer.h'
)

sources = ['main.cpp', 'player.cpp', lexer_cpp]

executable('sim',
  sources: sources,
  install: false
)
