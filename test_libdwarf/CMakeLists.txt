# プロジェクト設定
cmake_minimum_required(VERSION 3.29)
project(test_libdwarf CXX)
# 変数
#set(LIBDWARF_DIR /usr/local/libdwarf_092)
set(LIBDWARF_DIR D:/msys64/usr/local/libdwarf_092)
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
