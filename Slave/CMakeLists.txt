cmake_minimum_required (VERSION 2.8.10)

include_directories(
    ../asio-1.10.6/include
    ../Comm/Serial
    ../Comm/TCP
    ../Utils
    /usr/include
)

set(
    slave_SOURCES

    main.cpp
)

add_executable(
    slave

    ${slave_SOURCES}
)

target_link_libraries(
    slave

   -lpthread
   -lrt
)
