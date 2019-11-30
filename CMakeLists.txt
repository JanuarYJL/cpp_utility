cmake_minimum_required(VERSION 3.8)

# C++程序
project(utility CXX)

# 使用C++11标准
set(CMAKE_CXX_STANDARD 11)

# CMAKE_C_FLAGS/CMAKE_CXX_FLAGS
#编译设置 加载当前目录:当前目录下lib_so:当前目录上级目录下lib_so文件夹的so
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-deprecated -Wno-conversion-null -Wno-unused-function -Wl,-rpath,./:./lib_so:../lib_so")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-deprecated -Wno-conversion-null -Wno-unused-function -Wl,-rpath,./:./lib_so:../lib_so")

# 宏定义
# 多字节-D_MBCS -DMBCS
# Unicode -D_UNICODE -DUNICODE
# ASIO仅使用标准库 -DASIO_STANDALONE
add_definitions(
        -D_MBCS #多字节字符集
        -DMBCS #多字节字符集
        -DBOOST_ALL_DYN_LINK
)

# third_party
set(THIRD_PARTY_PATH ../third_party)

# 包含路径
include_directories(
        include/utility
        ${THIRD_PARTY_PATH}/asio-1.12.2/include
        ${THIRD_PARTY_PATH}/boost_1_71_0/include
        ${THIRD_PARTY_PATH}/protobuf-3.11.0/include
)

# src
aux_source_directory(src UTILITY_SRC_FILES)

# out dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib/debug)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib/debug)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib/release)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib/release)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release)

# library
add_library(utility ${UTILITY_SRC_FILES})
