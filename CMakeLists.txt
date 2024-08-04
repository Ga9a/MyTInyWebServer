cmake_minimum_required(VERSION 3.10)

project(MyTinyWebServer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall")

# 添加可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../out)

# 源文件列表
set(SOURCES
    src/main.cpp
    src/log.cpp
    src/webserver.cpp
    src/sql_pool.cpp
    src/epoller.cpp
    src/httpconnector.cpp
    src/buffer.cpp
)

# 头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/log)

# 获取源代码的根目录
get_filename_component(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
string(REPLACE "\\" "/" SOURCE_ROOT "${SOURCE_ROOT}")

# 将源代码根目录传递给编译器
add_compile_definitions(SOURCE_ROOT="${SOURCE_ROOT}")

# 添加可执行文件
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# 使用 mysql_config 获取 MySQL 的编译选项
execute_process(COMMAND mysql_config --cflags OUTPUT_VARIABLE MYSQL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --libs OUTPUT_VARIABLE MYSQL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

# 将 MySQL 的编译选项添加到编译器中
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${MYSQL_CFLAGS})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MYSQL_LIBS})

# 添加标准C++库链接
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE stdc++fs)