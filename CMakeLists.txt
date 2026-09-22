#指定CMake最低版本
cmake_minimum_required(VERSION 3.10)

#项目名称
project(IM_Server)

#使用C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#包含头文件的目录
include_directories(${CMAKE_SOURCE_DIR}/include)

#添加exe
add_executable(IMServer src/main_sever.cpp)