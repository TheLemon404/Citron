add_library(citron_assets SHARED)

target_compile_definitions(citron_assets PRIVATE CITRON_ASSETS_BUILD_DLL)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_assets PRIVATE ${src})
target_include_directories(citron_assets PUBLIC include)

target_compile_features(citron_assets PUBLIC cxx_std_26)
target_include_directories(citron_assets PUBLIC ${CMAKE_SOURCE_DIR}/vendor/cereal/include ${CMAKE_SOURCE_DIR}/vendor/cereal-yaml/include)
target_link_libraries(citron_assets PUBLIC EnTT yaml-cpp::yaml-cpp)
target_link_libraries(citron_assets PRIVATE citron_io citron_core)

citron_export_sdk_headers(citron_assets)
