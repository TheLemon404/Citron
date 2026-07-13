add_executable(citron_editor)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp src/*.hpp)

target_sources(citron_assets PRIVATE ${src})

target_link_libraries(citron_editor
PRIVATE
citron_assets
citron_audio
citron_core
citron_ecs
citron_graphics
citron_input
citron_io
citron_networking
citron_physics
citron_util
)
