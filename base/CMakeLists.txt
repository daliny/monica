# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.5)

PROJECT(MONICA)
# 添加编译器选项
add_compile_options(-g -std=c++11)
# 添加子目录
ADD_SUBDIRECTORY(tests)
# 指定文件位置
SET(LIBMONICA_BASE ThreadPool.cc socket.cc error.cc)
# 添加静态库
ADD_LIBRARY(monica_base ${LIBMONICA_BASE})
TARGET_LINK_LIBRARIES(monica_base pthread)
