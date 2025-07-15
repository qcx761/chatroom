cmake_minimum_required(VERSION 3.10)
project(client)

set(CMAKE_CXX_STANDARD 17)

# 添加头文件路径
include_directories(
    /usr/local/include       # hiredis, redis++
    /usr/include/mysql-cppconn-8 # 如果有此路径
)

# 添加库文件路径
link_directories(
    /usr/local/lib
)

add_executable(client
    main.cpp
    client.cpp
    menu.cpp
    account.cpp
    json.cpp
    msg.cpp
    ../threadpool/threadpool.cpp
    ../log/logger.cpp
)

target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../threadpool
    ../log
)

# 修改这里：把 mysqlcppconn8 改成 mysqlcppconn
target_link_libraries(client
    redis++          # Redis++ C++ client
    hiredis          # hiredis 是 redis++ 的底层依赖
    mysqlcppconn     # 修改这里，去掉 8 后缀
)
