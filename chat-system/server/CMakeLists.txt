cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_CXX_STANDARD 17)

# 添加头文件路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../threadpool
    /usr/local/include         # redis++ & hiredis
    /usr/include/mysql-cppconn-8  # MySQL C++ Connector headers
)

# 添加库文件路径
link_directories(
    /usr/local/lib             # redis++ & hiredis
    /usr/lib/x86_64-linux-gnu  # MySQL C++ Connector .so 路径
)

# server 可执行文件
add_executable(server
    main.cpp
    server.cpp
    subreactor.cpp
    json.cpp
    msg.cpp
    MessageQueue.cpp
    MySQLPool.cpp
    ../threadpool/threadpool.cpp
)

target_link_libraries(server
    redis++
    hiredis
    mysqlcppconn
)

# ftpserver 可执行文件
add_executable(ftpserver
    ftpmain.cpp
    ftpserver.cpp
    ../threadpool/threadpool.cpp
)




