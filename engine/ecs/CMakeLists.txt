add_library(citron_ecs SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)
file(GLOB_RECURSE inc CONFIGURE_DEPENDS  src/*.hpp)

target_sources(citron_ecs PRIVATE ${src})
target_include_directories(citron_ecs PUBLIC ${inc})

target_compile_features(citron_ecs PUBLIC cxx_std_26)
