add_executable(citron_editor)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp src/*.hpp)

target_sources(citron_editor PRIVATE ${src})

target_link_libraries(citron_editor
PRIVATE
citron_app
citron_assets
#citron_audio
citron_core
citron_ecs
citron_graphics
citron_input
imgui
yaml-cpp::yaml-cpp
citron_io
#citron_networking
#citron_physics
#citron_util
)

function(target_copy_shared_library target library)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:${library}>
		$<TARGET_FILE_DIR:${target}>
	)
endfunction()

target_copy_shared_library(citron_editor citron_app)
target_copy_shared_library(citron_editor citron_assets)
target_copy_shared_library(citron_editor citron_graphics)
target_copy_shared_library(citron_editor citron_core)
target_copy_shared_library(citron_editor citron_ecs)
target_copy_shared_library(citron_editor citron_input)
target_copy_shared_library(citron_editor citron_io)
target_copy_shared_library(citron_editor citron_assets)
target_copy_shared_library(citron_editor SDL3::SDL3)
target_copy_graphics_shared_dependencies(citron_editor)
