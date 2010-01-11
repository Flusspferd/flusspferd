# Place this in CMakeLists.txt
cmake_minimum_required(VERSION 2.6.4)
execute_process( COMMAND flusspferd --cmake OUTPUT_VARIABLE hippo_cmake)
include("${hippo_cmake}")

FILE(GLOB_RECURSE sources 
     "${CMAKE_SOURCE_DIR}/src/*.cpp" 
     "${CMAKE_SOURCE_DIR}/src/*.hpp")

add_definitions("-Wall")
flusspferd_plugin( "dummy_module" SOURCES ${sources})

