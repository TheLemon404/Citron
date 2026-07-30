cmake_minimum_required(VERSION 3.0...3.5)

project(citron VERSION 0.1 LANGUAGES CXX)
enable_language(C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(vendor)
add_subdirectory(engine)
add_subdirectory(editor)
