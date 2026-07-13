add_library(citron_assets SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)
file(GLOB_RECURSE inc CONFIGURE_DEPENDS  src/*.hpp)

target_sources(citron_assets PRIVATE ${src})
target_include_directories(citron_assets PUBLIC ${inc})

target_compile_features(citron_assets PUBLIC cxx_std_26)
