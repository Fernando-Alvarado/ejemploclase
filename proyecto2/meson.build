project('k-mst', 'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++20',
    'warning_level=3',
    'optimization=2'
  ]
)

inc = include_directories('include')

srcs = files(
  'src/main.cpp',
  'src/graph.cpp',
  'src/graph_reader.cpp',
  'src/pso.cpp'
)

omp_dep = dependency('openmp', required : true)

exe = executable('kmst', srcs,
  include_directories : inc,
  dependencies: [omp_dep],
  install : true
)

#subdir('tests')
