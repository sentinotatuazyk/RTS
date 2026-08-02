project('projektzdll', 'cpp',
  version: '1.0.0',
  default_options:[
    'cpp_std=c++20',
    'warning_level=3',
    'werror=false',
    'default_library=shared'
  ]
)

inc = include_directories('DLL')

sfml_graphics=dependency('sfml-graphics',required:true)
sfml_window=dependency('sfml-window', required:true)
sfml_system=dependency('sfml-system', required:true)
sfml_audio=dependency('sfml-system', required:true)
sfml_network=dependency('sfml-network', required:false)

sfml_deps=[sfml_graphics,sfml_window, sfml_system, sfml_audio]
if sfml_network.found()
    sfml_deps+=sfml_deps
endif
subdir('DLL')
subdir('projektzdll')
subdir('assets')
