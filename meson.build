project('crain', 'c', default_options : ['c_std=c99'])

src = ['main.c']

deps = [
  dependency('ncurses'),
]

executable('crain', sources : src, dependencies: deps, install : true)
