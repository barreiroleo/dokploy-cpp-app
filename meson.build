project(
  'hello-app',
  'cpp',
  version: '1.0.0',
  default_options: ['cpp_std=c++23', 'warning_level=3', 'werror=false'],
)

# Get cpp-httplib dependency from WrapDB
cpp_httplib_dep = dependency('cpp-httplib')

sources = files('src/main.cpp')

executable('hello-app', sources, dependencies: [cpp_httplib_dep], install: true)
