add_library(citron_audio SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_audio PRIVATE ${src})
target_include_directories(citron_audio PUBLIC include)

target_compile_features(citron_audio PUBLIC cxx_std_26)
