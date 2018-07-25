# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.5)

# 添加编译器选项
add_compile_options(-g -std=c++11)
# 添加头文件搜索路径
include_directories(..)

# 指定生成目标
add_executable(BlockingQueue_test BlockingQueue_test.cc)

# 添加链接库
target_link_libraries(BlockingQueue_test pthread)

add_executable(BoundedBlockingQueue_test BoundedBlockingQueue_test.cc)
target_link_libraries(BoundedBlockingQueue_test pthread)

add_executable(ThreadPool_test ThreadPool_test.cc ../ThreadPool.cc)
target_link_libraries(ThreadPool_test pthread)

add_executable(OneThread_test OneThread_test.cc)
target_link_libraries(ThreadPool_test pthread)
