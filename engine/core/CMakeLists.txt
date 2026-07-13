add_library(citron_core SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)
file(GLOB_RECURSE inc CONFIGURE_DEPENDS  src/*.hpp)

target_sources(citron_core PRIVATE ${src})
target_include_directories(citron_core PUBLIC ${inc})

target_compile_features(citron_core PUBLIC cxx_std_26)
