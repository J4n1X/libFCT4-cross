# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("FCT-cross")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# test that filesystem header actually is there and works
try_compile(HAS_FS "${CMAKE_BINARY_DIR}/temp" 
"${CMAKE_SOURCE_DIR}/tests/has_filesystem.cpp" 
            CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
            LINK_LIBRARIES stdc++fs)
if(HAS_FS)
    message(STATUS "Compiler has filesystem support")
else()
#   .... You could also try searching for boost::filesystem here.
    message(FATAL_ERROR "Compiler is missing filesystem capabilities")
endif(HAS_FS)

# Include sub-projects.
add_subdirectory ("FCT-cross")
