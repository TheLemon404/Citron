add_library(citron_input SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_input PRIVATE ${src})
target_include_directories(citron_input PUBLIC include)

target_compile_features(citron_input PUBLIC cxx_std_26)
target_link_libraries(citron_input PUBLIC SDL3::SDL3)
target_link_libraries(citron_input PRIVATE citron_core)
