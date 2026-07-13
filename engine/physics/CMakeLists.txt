add_library(citron_physics SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)
file(GLOB_RECURSE inc CONFIGURE_DEPENDS  src/*.hpp)

target_sources(citron_physics PRIVATE ${src})
target_include_directories(citron_physics PUBLIC ${inc})

target_compile_features(citron_physics PUBLIC cxx_std_26)
