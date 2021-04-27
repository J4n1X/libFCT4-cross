# CMakeList.txt : CMake project for libFCT-cpp-cross, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

add_library(libFCT STATIC "libFCT.h" "FctArchive.cpp" "FctArchive.h" "FileParser.cpp" "FileParser.h" "FSoperations.cpp" "FSoperations.h")

# Add source to this project's executable.
add_executable (libFCT-cross "libFCT-cross.cpp")
set_property(TARGET libFCT-cross PROPERTY CXX_STANDARD 17)
set_property(TARGET libFCT PROPERTY CXX_STANDARD 17)
if(LINK_FILESYSTEM)
	target_link_libraries(${PROJECT_NAME} stdc++fs
						  libFCT)
else()
	target_link_libraries(${PROJECT_NAME} libFCT)
endif()
# TODO: Add tests and install targets if needed.
