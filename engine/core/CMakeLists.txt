add_library(citron_core SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_core PRIVATE ${src})
target_include_directories(citron_core PUBLIC include)

target_compile_features(citron_core PUBLIC cxx_std_26)

target_link_libraries(citron_core PUBLIC spdlog::spdlog)
