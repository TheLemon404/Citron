add_library(citron_ecs SHARED)

target_compile_definitions(citron_ecs PRIVATE CITRON_ECS_BUILD_DLL)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_ecs PRIVATE ${src})
target_include_directories(citron_ecs PUBLIC include)

target_compile_features(citron_ecs PUBLIC cxx_std_26)
target_link_libraries(citron_ecs PUBLIC yaml-cpp::yaml-cpp)
target_link_libraries(citron_ecs PRIVATE citron_core citron_assets EnTT citron_io citron_graphics citron_util)
