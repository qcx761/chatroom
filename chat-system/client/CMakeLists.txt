# cmake_minimum_required(VERSION 3.10)
# project(client)

# set(CMAKE_CXX_STANDARD 17)

# # 只保留 readline 相关头文件路径
# include_directories(
#     /usr/include/readline
# )

# # 添加 libprofiler 库文件路径
# link_directories(
#     /lib/x86_64-linux-gnu
# )

# add_executable(client
#     main.cpp
#     client.cpp
#     menu.cpp
#     account.cpp
#     json.cpp
#     msg.cpp
# )

# target_link_libraries(client
#     readline
#     history
#     profiler
# )








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

