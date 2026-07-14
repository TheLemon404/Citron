add_executable(citron_editor)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp src/*.hpp)

target_sources(citron_editor PRIVATE ${src})

target_link_libraries(citron_editor
PRIVATE
#citron_assets
#citron_audio
citron_core
#citron_ecs
#citron_graphics
#citron_input
citron_io
#citron_networking
#citron_physics
#citron_util
)

add_custom_command(TARGET citron_editor POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  $<TARGET_FILE:citron_core>
  $<TARGET_FILE_DIR:citron_editor>
)
add_custom_command(TARGET citron_editor POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  $<TARGET_FILE:citron_io>
  $<TARGET_FILE_DIR:citron_editor>
)
add_custom_command(TARGET citron_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:citron_editor>
)
