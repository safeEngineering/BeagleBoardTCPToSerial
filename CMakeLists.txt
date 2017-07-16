cmake_minimum_required (VERSION 2.8.10)

project (Serial-TCP)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to 'Debug' as none was specified.")
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_C_FLAGS}")


set(
    CMAKE_CXX_FLAGS

    "${CMAKE_CXX_FLAGS} ${COMMON_C_FLAGS}\
     -pthread -std=c++11 -O2"
)

add_subdirectory(Master)

