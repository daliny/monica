# 添加头文件搜索路径
include_directories(..)

# 指定生成目标
add_executable(BlockingQueue_test BlockingQueue_test.cc)
# 添加链接库
target_link_libraries(BlockingQueue_test monica_base)

add_executable(BoundedBlockingQueue_test BoundedBlockingQueue_test.cc)
target_link_libraries(BoundedBlockingQueue_test monica_base)

add_executable(ThreadPool_test ThreadPool_test.cc)
target_link_libraries(ThreadPool_test monica_base)

add_executable(OneThread_test OneThread_test.cc)
target_link_libraries(ThreadPool_test monica_base)

add_executable(Socket_test socket_test.cc)
target_link_libraries(Socket_test monica_base)

add_executable(Acceptor_test acceptor_test.cc)
target_link_libraries(Acceptor_test monica_base)

add_executable(Epoll_test epoll_test.cc)
target_link_libraries(Epoll_test monica_base)

add_executable(TcpConnection_test TcpConnection_test.cc)
target_link_libraries(TcpConnection_test monica_base)
