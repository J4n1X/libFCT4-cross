cmake_minimum_required (VERSION 3.8)

add_library(FCT "libFCT.h" "FctArchive.cpp" "FctArchive.h" "FSoperations.cpp" "FSoperations.h" "FileParser.cpp" "FileParser.h")

# test that filesystem header actually is there and works
try_compile(HAS_FS "${CMAKE_BINARY_DIR}/temp" 
"${CMAKE_SOURCE_DIR}/tests/has_filesystem.cpp" 
            CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
            LINK_LIBRARIES stdc++fs)
if(HAS_FS)
    message(STATUS "Compiler has filesystem support")
    target_link_libraries(FCT stdc++fs)
else()
#   .... You could also try searching for boost::filesystem here.
    #message(FATAL_ERROR "Compiler is missing filesystem capabilities")
endif(HAS_FS)
