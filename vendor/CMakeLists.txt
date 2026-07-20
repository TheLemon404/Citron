add_library(citron_vendor INTERFACE)

set(DAWN_FETCH_DEPENDENCIES ON)

add_subdirectory(Box3D)
add_subdirectory(EnTT)
add_subdirectory(GLM)
add_subdirectory(SDL)
add_subdirectory(WebGPU)
add_subdirectory(WebGPU_SDL)
add_subdirectory(spdlog)
add_subdirectory(imgui)
add_subdirectory(nativefiledialog)
add_subdirectory(yaml-cpp)
