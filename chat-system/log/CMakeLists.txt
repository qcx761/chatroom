# log/CMakeLists.txt
add_library(log STATIC logger.cpp)

target_include_directories(log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
