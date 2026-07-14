add_library(citron_util SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_util PRIVATE ${src})
target_include_directories(citron_util PUBLIC include)

target_compile_features(citron_util PUBLIC cxx_std_26)
