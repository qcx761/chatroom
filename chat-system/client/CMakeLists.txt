cmake_minimum_required(VERSION 3.10)
project(client)

set(CMAKE_CXX_STANDARD 17)

add_executable(client
    main.cpp
    client.cpp
    menu.cpp
    account.cpp
    ../threadpool/threadpool.cpp
    ../log/logger.cpp          # 添加日志实现文件
)

target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../threadpool
    ../log                     # 添加日志头文件目录
)
