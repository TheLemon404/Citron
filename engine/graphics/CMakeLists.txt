add_library(citron_graphics SHARED)

set_target_properties(citron_graphics PROPERTIES CXX_STANDARD 20)
target_compile_definitions(citron_graphics PRIVATE CITRON_GRAPHICS_BUILD_DLL)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_graphics PRIVATE ${src})
target_include_directories(citron_graphics PUBLIC include PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/vendor/miniray/build>)

target_compile_features(citron_graphics PUBLIC cxx_std_26)
target_link_libraries(citron_graphics PUBLIC webgpu glm::glm assimp stb citron_util)
target_link_libraries(citron_graphics PRIVATE citron_core miniray nlohmann_json citron_assets citron_io)

function(target_copy_graphics_shared_dependencies target)
	target_copy_webgpu_binaries(${target})
endfunction()
