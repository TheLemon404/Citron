add_library(citron_input SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)
file(GLOB_RECURSE inc CONFIGURE_DEPENDS  src/*.hpp)

target_sources(citron_input PRIVATE ${src})
target_include_directories(citron_input PUBLIC ${inc})

target_compile_features(citron_input PUBLIC cxx_std_26)
