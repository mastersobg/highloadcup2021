cmake_minimum_required(VERSION 3.16.3)
project(highloadcup2021)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_TYPE "" CACHE STRING "")
set(COMMIT_HASH "" CACHE STRING "")

find_package(RapidJSON REQUIRED)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

file(GLOB TARGET_SRC src/*.cpp)
add_executable(highloadcup2021 ${TARGET_SRC})

# common cxx flags
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -Wformat=2 -Wfloat-equal -Wconversion -Warith-conversion -Wshadow -Wsign-conversion -Wlogical-op -Wdouble-promotion -Wshift-overflow=2  -Wduplicated-cond -Wcast-qual -Wcast-align -O2 -pedantic -pipe")

message(STATUS "build type ${BUILD_TYPE}")
add_definitions("-DBUILD_TYPE=\"${BUILD_TYPE}\"")
add_definitions("-DCOMMIT_HASH=\"${COMMIT_HASH}\"")

if (BUILD_TYPE STREQUAL "release")
elseif (BUILD_TYPE STREQUAL "debug")
    string(APPEND CMAKE_CXX_FLAGS " -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -D_HLC_DEBUG -g")
elseif (BUILD_TYPE STREQUAL "debug-thread")
    string(APPEND CMAKE_CXX_FLAGS " -fno-omit-frame-pointer -fsanitize=thread -g")
elseif (BUILD_TYPE STREQUAL "profile-memory")
    # TODO
    string(APPEND CMAKE_CXX_FLAGS " -g -fno-omit-frame-pointer")
elseif (BUILD_TYPE STREQUAL "profile-cpu")
    # uncomment to see all functions on perf report
    string(APPEND CMAKE_CXX_FLAGS " -g -fno-omit-frame-pointer")
elseif (BUILD_TYPE STREQUAL "dump")
    string(APPEND CMAKE_CXX_FLAGS " -g -fno-omit-frame-pointer")
elseif (BUILD_TYPE STREQUAL "lock")
    string(APPEND CMAKE_CXX_FLAGS " -g -fno-omit-frame-pointer -rdynamic")
elseif (BUILD_TYPE STREQUAL "clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wformat=2 -Wfloat-equal -Wconversion  -Wshadow -Wsign-conversion  -Wdouble-promotion    -Wcast-qual -Wcast-align -O0 -pedantic -pipe -fno-omit-frame-pointer  -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_HLC_DEBUG -g")
else ()
    message(FATAL_ERROR "wrong build type " ${BUILD_TYPE})
endif ()

message(STATUS "CXX flags: ${CMAKE_CXX_FLAGS}")

target_link_libraries(highloadcup2021 pthread curl)
