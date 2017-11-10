cmake_minimum_required (VERSION 2.8.10)

include_directories(
    ../asio-1.10.6/include
    ../spdlog/include
    ../Comm/Log
    ../Comm/Serial
    ../Comm/TCP
    ../Utils
    ../libbbb/src/
    /usr/include
)

set(
    master_SOURCES

    main.cpp
#   ../easyBlack/src/memGPIO.cpp
#   ../easyBlack/src/utils.cpp
    ../libbbb/src/bbb.c
)

add_executable(
    master

    ${master_SOURCES}
)

target_link_libraries(
    master

   -lpthread
   -lrt
)
