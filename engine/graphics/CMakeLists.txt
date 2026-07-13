add_library(citron_graphics SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)
file(GLOB_RECURSE inc CONFIGURE_DEPENDS  src/*.hpp)

target_sources(citron_graphics PRIVATE ${src})
target_include_directories(citron_graphics PUBLIC ${inc})

target_compile_features(citron_graphics PUBLIC cxx_std_26)
