# CMakeList.txt : CMake project for libFCT-cpp-cross, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

add_subdirectory(libFCT)

add_executable (FCT-UI "libFCT-cross.cpp")

add_dependencies(FCT-UI FCT)

set_property(TARGET FCT-UI PROPERTY CXX_STANDARD 17)

target_link_libraries(FCT-UI FCT)