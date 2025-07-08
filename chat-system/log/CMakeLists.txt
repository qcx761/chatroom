# log/CMakeLists.txt
add_library(log STATIC logger.cpp logger.h)

target_include_directories(log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
