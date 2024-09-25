# プロジェクト設定
cmake_minimum_required(VERSION 3.29)
project(test_libdwarf CXX)

# dllを結合するとき：-static -lgcc -lstdc++ -lz -lzstd
# より最適化するとき： -s -march=native
set(CMAKE_CXX_FLAGS "-std=gnu++20 -static -lgcc -lstdc++ -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wendif-labels -Wfloat-equal -Winit-self -Winline -Wlogical-op -Wmissing-include-dirs -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wswitch-default -Wswitch-enum -Wunsafe-loop-optimizations -Wvariadic-macros -Wwrite-strings ")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
# set(CMAKE_BUILD_TYPE Debug)

# 変数
#set(LIBDWARF_DIR D:/msys64/usr/local/libdwarf_092)
set(LIBDWARF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdwarf)
set(LINKLIB_DIR D:/msys64/ucrt64/lib)
#
include_directories(${LIBDWARF_DIR}/include)
link_directories(${LIBDWARF_DIR}/bin ${LIBDWARF_DIR}/lib)

#
#add_library(libdwarf SHARED IMPORTED)
add_library(libdwarf STATIC IMPORTED)
set_target_properties(libdwarf PROPERTIES
    IMPORTED_IMPLIB ${LIBDWARF_DIR}/lib/libdwarf.dll.a)
set_target_properties(libdwarf PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${LIBDWARF_DIR}/include)
set_target_properties(libdwarf PROPERTIES
    IMPORTED_LOCATION ${LIBDWARF_DIR}/lib/libdwarf.a)

add_library(libz STATIC IMPORTED)
set_target_properties(libz PROPERTIES
    IMPORTED_IMPLIB ${LINKLIB_DIR}/lib/libz.dll.a)
#set_target_properties(libz PROPERTIES
#    INTERFACE_INCLUDE_DIRECTORIES ${LINKLIB_DIR}/include)
set_target_properties(libz PROPERTIES
    IMPORTED_LOCATION ${LINKLIB_DIR}/libz.a)

add_library(libzstd STATIC IMPORTED)
set_target_properties(libzstd PROPERTIES
    IMPORTED_IMPLIB ${LINKLIB_DIR}/lib/libzstd.dll.a)
#set_target_properties(libz PROPERTIES
#    INTERFACE_INCLUDE_DIRECTORIES ${LINKLIB_DIR}/include)
set_target_properties(libzstd PROPERTIES
    IMPORTED_LOCATION ${LINKLIB_DIR}/libzstd.a)

#
add_executable(test_libdwarf main.cpp)
target_link_libraries(test_libdwarf libdwarf libz libzstd)
#target_include_directories(test_libdwarf ${LIBDWARF_DIR}/include)
#
set_target_properties(test_libdwarf PROPERTIES
    IMPORTED_LOCATION ${LIBDWARF_DIR}/bin/libdwarf.dll)
