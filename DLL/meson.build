dll_sources=files(
  'buildings.cpp',
  'enemy.cpp',
  'game.cpp',
  'Klasa.cpp',
  'map.cpp',
  'menu.cpp',
  'PerlinNoise.cpp',
  'player.cpp',
  'settings.cpp',
  'ui_manager.cpp',
  'unit.cpp',
  'utils.cpp'
)

game_lib = shared_library('game',
  dll_sources,
  include_directories : inc,
  dependencies : sfml_deps,
  install : true,
  cpp_args : ['-DGAME_BUILD_DLL']
)

# Deklaracja zależności — inne targety mogą jej użyć
game_dep = declare_dependency(
  link_with : game_lib,
  include_directories : inc,
  dependencies : sfml_deps
)
