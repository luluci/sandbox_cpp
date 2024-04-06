# プロジェクト設定
cmake_minimum_required(VERSION 3.29)
project(test_libdwarf CXX)

set(CMAKE_CXX_FLAGS "-std=gnu++20 -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wendif-labels -Wfloat-equal -Winit-self -Winline -Wlogical-op -Wmissing-include-dirs -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wswitch-default -Wswitch-enum -Wunsafe-loop-optimizations -Wvariadic-macros -Wwrite-strings ")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -s -DNDEBUG -march=native")
set(CMAKE_BUILD_TYPE Debug)

# 変数
#set(LIBDWARF_DIR D:/msys64/usr/local/libdwarf_092)
set(LIBDWARF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdwarf)
#
include_directories(${LIBDWARF_DIR}/include)
link_directories(${LIBDWARF_DIR}/bin ${LIBDWARF_DIR}/lib)

#
add_library(libdwarf SHARED IMPORTED)
set_target_properties(libdwarf PROPERTIES
    IMPORTED_IMPLIB ${LIBDWARF_DIR}/lib/libdwarf.dll.a)
set_target_properties(libdwarf PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${LIBDWARF_DIR}/include)


#
add_executable(test_libdwarf main.cpp)
target_link_libraries(test_libdwarf libdwarf)
#target_include_directories(test_libdwarf ${LIBDWARF_DIR}/include)
#
set_target_properties(test_libdwarf PROPERTIES
    IMPORTED_LOCATION ${LIBDWARF_DIR}/bin/libdwarf.dll)
