project(
  'vulkan-test',
  'cpp',
  version: '0.0.0',
  meson_version: '1.0.0',

  default_options: [
    'cpp_std=c++20',
    'default_library=static'
  ]
)

if get_option('buildtype') == 'debug'
  add_project_arguments(['-DDEBUG'], language: 'cpp')
endif 

executable(
  'main',
  sources: ['src/main.cpp'],
  dependencies: [
    dependency('vulkan'),
    dependency('glfw3')
  ],
)