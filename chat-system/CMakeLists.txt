cmake_minimum_required(VERSION 3.10)
project(ChatSystem)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(
    src
    src/common
    src/db
    src/handler
    src/model
    src/net
    src/server
    src/reactor
    src/threadpool
    include
)

# 编译源文件
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

# 可执行文件
add_executable(chat_server ${SOURCES})

# 链接 hiredis / mysql / pthread
target_link_libraries(chat_server
    pthread
    hiredis
    mysqlclient
)
