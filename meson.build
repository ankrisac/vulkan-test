project(
  'vulkan-test',
  'cpp',
  version: '0.0.0',
  meson_version: '1.0.0',
  license: 'GNU GPLv3',

  default_options: [
    'cpp_std=c++20',
    'default_library=static',

    'warning_level=everything',
    'werror=true'
  ]
)

cc = meson.get_compiler('cpp')

if cc.get_id() == 'msvc'
  add_project_arguments(
    ['/diagnostics:caret'],
    language: 'cpp'
  )
endif

if get_option('buildtype') == 'debug'
  add_project_arguments(['-DDEBUG'], language: 'cpp')
endif 

executable(
  'main',
  sources: [
    'src/main.cpp',
    'src/instance.cpp'
  ],
  dependencies: [
    dependency('vulkan'),
    dependency('glfw3')
  ]
)