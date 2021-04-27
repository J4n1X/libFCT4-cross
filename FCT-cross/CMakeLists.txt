# CMakeList.txt : CMake project for libFCT-cpp-cross, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

add_library(FCT STATIC "libFCT.h" "FctArchive.cpp" "FctArchive.h" "FileParser.cpp" "FileParser.h" "FSoperations.cpp" "FSoperations.h")

# Add source to this project's executable.
add_executable (FCT-UI "libFCT-cross.cpp")
set_property(TARGET FCT-UI PROPERTY CXX_STANDARD 17)
set_property(TARGET FCT PROPERTY CXX_STANDARD 17)
if(LINK_FILESYSTEM)
	target_link_libraries(FCT stdc++fs)
	target_link_libraries(FCT-UI stdc++fs FCT)
else()
	target_link_libraries(FCT-UI FCT)
endif()
# TODO: Add tests and install targets if needed.
