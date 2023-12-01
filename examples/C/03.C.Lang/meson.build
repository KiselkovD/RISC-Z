project('Lang', 'c')              # Lang — имя проекта,
                                  # c — язык программирования

if get_option('buildtype') == 'debug'
  add_project_arguments('-DDEBUG',  '-g3', '-std=c2x', language : 'c')
endif

if get_option('buildtype') == 'release'
  add_project_arguments('-DNDEBUG', '-O3', '-std=c2x', language : 'c')
endif

minipeg_gen = generator(find_program('minipeg'),
  output  : '@BASENAME@.c',
  arguments : ['-o', '@OUTPUT@', '@INPUT@']
)
minipeg_gen_src = minipeg_gen.process('basic.peg')

executable('lang', 'main.c', minipeg_gen_src)
