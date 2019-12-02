cmake_minimum_required(VERSION 3.1.0)

project(mwvd)

option(BUILD_VIEW "Build with graphical user interface" OFF)
option(BUILD_LOG "Build with logging functionality" OFF)

set (Tutorial_VERSION_MAJOR 0)
set (Tutorial_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wfloat-equal -Wundef -Wpointer-arith")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wwrite-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wredundant-decls -Wchar-subscripts -Wcomment -Wformat=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wwrite-strings -Wmissing-declarations -Wredundant-decls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-enum")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self -Wmissing-field-initializers")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Waddress -Wmissing-noreturn -Wstrict-overflow=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Warray-bounds")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

if(BUILD_VIEW)
    add_definitions(-DENABLE_VIEW)
endif(BUILD_VIEW)

if(BUILD_LOG)
    add_definitions(-DENABLE_LOG)
endif(BUILD_LOG)

add_subdirectory(src bin)
