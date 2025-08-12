cmake_minimum_required(VERSION 3.10)
project(client)

set(CMAKE_CXX_STANDARD 17)

# 只保留 readline 相关头文件路径
include_directories(
    /usr/include/readline
)

add_executable(client
    main.cpp
    client.cpp
    menu.cpp
    account.cpp
    json.cpp
    msg.cpp
)

target_link_libraries(client
    readline
    history
)

