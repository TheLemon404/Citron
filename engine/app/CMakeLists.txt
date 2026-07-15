add_library(citron_app SHARED)

file(GLOB_RECURSE src CONFIGURE_DEPENDS  src/*.cpp)

target_sources(citron_app PRIVATE ${src})
target_include_directories(citron_app PUBLIC include)

target_compile_features(citron_app PUBLIC cxx_std_26)

target_link_libraries(citron_app
PUBLIC
citron_core
citron_input
)
