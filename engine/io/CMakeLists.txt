add_library(citron_io SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)
file(GLOB_RECURSE inc CONFIGURE_DEPENDS  src/*.hpp)

target_sources(citron_io PRIVATE ${src})
target_include_directories(citron_io PUBLIC ${inc})

target_compile_features(citron_io PUBLIC cxx_std_26)
