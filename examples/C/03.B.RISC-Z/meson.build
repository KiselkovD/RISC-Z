project('RISC-Z', 'c')            # tutorial — имя проекта,
                                  # c — язык программирования

if get_option('buildtype') == 'debug'
  add_project_arguments('-DDEBUG',  '-g3', language : 'c')
endif

if get_option('buildtype') == 'release'
  add_project_arguments('-DNDEBUG', '-O3', language : 'c')
endif

executable('risczemu', 'main.c', 'risc-z.c', 'memory.c', 'ecall.c')
