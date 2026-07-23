add_library(citron_assets SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_assets PRIVATE ${src})
target_include_directories(citron_assets PUBLIC include)

target_compile_features(citron_assets PUBLIC cxx_std_26)
target_link_libraries(citron_assets PUBLIC EnTT alpaca)
target_link_libraries(citron_assets PRIVATE citron_io citron_core)
