cmake_minimum_required(VERSION 2.6)
project(cds)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(cds cds/hashtable.c)
install(TARGETS cds DESTINATION bin)
install(FILES cds/version.h cds/hashtable.h DESTINATION include/cds)
