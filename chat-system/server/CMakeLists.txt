cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_CXX_STANDARD 17)

# 添加头文件路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../threadpool
    ../log
    /usr/local/include         # redis++ & hiredis
    /usr/include/mysql-cppconn-8  # MySQL C++ Connector headers（路径可能不同）
)

# 添加库文件路径
link_directories(
    /usr/local/lib             # redis++ & hiredis
    /usr/lib/x86_64-linux-gnu  # MySQL C++ Connector .so 路径
)

add_executable(server
    main.cpp
    server.cpp
    subreactor.cpp
    json.cpp
    msg.cpp
    ../threadpool/threadpool.cpp
    ../log/logger.cpp
)

# 链接 redis++、hiredis、mysqlcppconn
target_link_libraries(server
    redis++
    hiredis
    mysqlcppconn
)
