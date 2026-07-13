add_library(citron_networking SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)
file(GLOB_RECURSE inc CONFIGURE_DEPENDS  src/*.hpp)

target_sources(citron_networking PRIVATE ${src})
target_include_directories(citron_networking PUBLIC ${inc})

target_compile_features(citron_networking PUBLIC cxx_std_26)
