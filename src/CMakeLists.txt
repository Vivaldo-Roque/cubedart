# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(cubedart_library VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cubedart SHARED
  "cubedart.cpp"
  "cubecpp/src/cube.cpp"
  "cubecpp/src/solver.cpp"
  "cubecpp/src/obfuscate.cpp"
)

target_include_directories(cubedart PRIVATE "cubecpp/include")

set_target_properties(cubedart PROPERTIES
  PUBLIC_HEADER cubedart.h
  OUTPUT_NAME "cubedart"
)

target_compile_definitions(cubedart PUBLIC DART_SHARED_LIB)

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(cubedart PRIVATE "-Wl,-z,max-page-size=16384")
endif()

