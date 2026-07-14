add_library(citron_io SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_io PRIVATE ${src})
target_include_directories(citron_io PUBLIC include)

target_compile_features(citron_io PUBLIC cxx_std_26)

target_link_libraries(citron_io PUBLIC SDL3::SDL3)
target_link_libraries(citron_io PRIVATE citron_core)
