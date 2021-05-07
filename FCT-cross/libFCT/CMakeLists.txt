cmake_minimum_required (VERSION 3.8)

add_library(FCT "libFCT.h" "FctArchive.cpp" "FctArchive.h" "FSoperations.cpp" "FSoperations.h" "FileParser.cpp" "FileParser.h")

target_link_libraries(FCT stdc++fs)
