cmake_minimum_required(VERSION 3.16.3)
project(highloadcup2021)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_TYPE "" CACHE STRING "")
set(COMMIT_HASH "" CACHE STRING "")

find_package(RapidJSON REQUIRED)

file(GLOB TARGET_SRC src/lib/*.cpp)
add_library(highloadcup2021_lib ${TARGET_SRC})
target_link_libraries(highloadcup2021_lib curl)
target_include_directories(highloadcup2021_lib PUBLIC ${RAPIDJSON_INCLUDE_DIRS})
add_executable(highloadcup2021 src/main.cpp)
target_link_libraries(highloadcup2021 highloadcup2021_lib)


# common cxx flags
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -Wformat=2 -Wfloat-equal -Wconversion -Warith-conversion -Wshadow -Wsign-conversion -Wlogical-op -Wdouble-promotion -Wshift-overflow=2  -Wduplicated-cond -Wcast-qual -Wcast-align  -pedantic -pipe")

message(STATUS "build type ${BUILD_TYPE}")
add_definitions("-DBUILD_TYPE=\"${BUILD_TYPE}\"")
add_definitions("-DCOMMIT_HASH=\"${COMMIT_HASH}\"")

if (BUILD_TYPE STREQUAL "release")
    string(APPEND CMAKE_CXX_FLAGS " -O2")
elseif (BUILD_TYPE STREQUAL "debug")
    string(APPEND CMAKE_CXX_FLAGS " -O0 -fno-omit-frame-pointer -D_HLC_DEBUG -g")
elseif (BUILD_TYPE STREQUAL "dev")
    string(APPEND CMAKE_CXX_FLAGS " -O2 -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -D_HLC_DEBUG -g")
elseif (BUILD_TYPE STREQUAL "dev-thread")
    string(APPEND CMAKE_CXX_FLAGS " -O2 -fno-omit-frame-pointer -fsanitize=thread -g")
elseif (BUILD_TYPE STREQUAL "profile-memory")
    string(APPEND CMAKE_CXX_FLAGS " -O2 -g -fno-omit-frame-pointer")
elseif (BUILD_TYPE STREQUAL "profile-cpu")
    # uncomment to see all functions on perf report
    string(APPEND CMAKE_CXX_FLAGS " -O2 -g -fno-omit-frame-pointer")
elseif (BUILD_TYPE STREQUAL "dump")
    string(APPEND CMAKE_CXX_FLAGS " -O2 -g -fno-omit-frame-pointer")
elseif (BUILD_TYPE STREQUAL "lock")
    string(APPEND CMAKE_CXX_FLAGS " -O2 -g -fno-omit-frame-pointer -rdynamic")
elseif (BUILD_TYPE STREQUAL "clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wformat=2 -Wfloat-equal -Wconversion  -Wshadow -Wsign-conversion  -Wdouble-promotion -Wcast-qual -Wcast-align -O0 -pedantic -pipe -fno-omit-frame-pointer  -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_HLC_DEBUG -g")

else ()
    message(FATAL_ERROR "wrong build type " ${BUILD_TYPE})
endif ()

message(STATUS "CXX flags: ${CMAKE_CXX_FLAGS}")


include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/6c5c4554ac218a8e19168edc121b1ad232015185.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB TARGET_TEST_SRC src/test/*.cpp)
message(STATUS "Test files: ${TARGET_TEST_SRC}")
add_executable(
        highloadcup2021_tests
        ${TARGET_TEST_SRC}
)
target_link_libraries(
        highloadcup2021_tests
        highloadcup2021_lib
        gtest_main
        gmock_main
)
target_include_directories(highloadcup2021_tests PUBLIC src/lib/)

include(GoogleTest)
gtest_discover_tests(highloadcup2021_tests)