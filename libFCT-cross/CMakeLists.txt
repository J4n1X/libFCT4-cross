# CMakeList.txt : CMake project for libFCT-cpp-cross, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
# Add source to this project's executable.
add_executable (libFCT-cross "libFCT-cross.cpp" "FctArchive.cpp" "FctArchive.h" "FileParser.cpp" "FileParser.h"  "libFCT.h" "FSoperations.cpp" "FSoperations.h" )
set_property(TARGET libFCT-cross PROPERTY CXX_STANDARD 17)
if(LINK_FILESYSTEM)
	target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()
# TODO: Add tests and install targets if needed.
