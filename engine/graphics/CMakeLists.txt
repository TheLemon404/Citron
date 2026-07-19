add_library(citron_graphics SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_graphics PRIVATE ${src})
target_include_directories(citron_graphics PUBLIC include)

target_compile_features(citron_graphics PUBLIC cxx_std_26)
target_link_libraries(citron_graphics PUBLIC webgpu sdl3webgpu)
target_link_libraries(citron_graphics PRIVATE citron_core citron_assets)

function(target_copy_graphics_shared_dependencies target)
	target_copy_webgpu_binaries(${target})
endfunction()
