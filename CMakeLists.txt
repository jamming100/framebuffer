# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

message(STATUS "******************************************************************")
message(STATUS "CMAKE_SOURCE_DIR = " ${CMAKE_SOURCE_DIR})
message(STATUS "******************************************************************")

# 项目信息
project (fb)
#toolchain-wrapper
set(CMAKE_C_COMPILER "${CMAKE_SOURCE_DIR}/../../buildroot/output/rockchip_rk3399/host/usr/bin/aarch64-buildroot-linux-gnu-cc.br_real") 
set(CMAKE_CXX_COMPILER "${CMAKE_SOURCE_DIR}/../../linux/buildroot/output/rockchip_rk3399/host/usr/bin/aarch64-buildroot-linux-gnu-g++.br_real")

#1.搜集指定路径下的源文件，将输出结果列表储存在指定的变量中。
aux_source_directory(${CMAKE_SOURCE_DIR}/  BASE_SRC)
#aux_source_directory(${CMAKE_SOURCE_DIR}/src/main/cpp/tool TOOL_SRC)

#2.设置ALL_SRCS变量,并把源文件赋值给它
set(ALL_SRCS ${BASE_SRC} )
#set(ALL_SRCS ${BASE_SRC} ${TOOL_SRC} )

# //生成动态库
#ADD_LIBRARY(ObCamera SHARED ${ALL_SRCS})
# 指定生成目标
add_executable(fb ${ALL_SRCS})
