cmake_minimum_required (VERSION 2.8.10)

include_directories(
    ../asio-1.10.6/include
    ../Comm/Serial
    ../Comm/TCP
    ../Utils
    /usr/include
)

set(
    submaster_SOURCES

    main.cpp
)

add_executable(
    submaster

    ${submaster_SOURCES}
)

target_link_libraries(
    submaster

   -lpthread
   -lrt
)
