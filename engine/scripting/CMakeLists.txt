add_library(citron_scripting SHARED)

target_compile_definitions(citron_scripting PRIVATE CITRON_SCRIPTING_BUILD_DLL)

file(GLOB_RECURSE src CONFIGURE_DEPENDS src/*.cpp)

target_sources(citron_scripting PRIVATE ${src})
target_include_directories(citron_scripting PUBLIC include)

target_compile_features(citron_scripting PUBLIC cxx_std_26)

target_link_libraries(citron_scripting PRIVATE citron_core citron_ecs citron_graphics citron_input citron_assets citron_util)
