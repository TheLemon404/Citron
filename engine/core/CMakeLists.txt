add_library(citron_core SHARED)

target_compile_definitions(citron_core PRIVATE CITRON_CORE_BUILD_DLL)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_core PRIVATE ${src})
target_include_directories(citron_core PUBLIC include)

target_compile_features(citron_core PUBLIC cxx_std_26)

set(SDL_SHARED ON CACHE BOOL "Build shared library" FORCE)

target_link_libraries(citron_core
PUBLIC
spdlog::spdlog
SDL3::SDL3
SDL3_image::SDL3_image
sdl3webgpu
)

target_link_libraries(citron_core PRIVATE webgpu)
