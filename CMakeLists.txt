cmake_minimum_required(VERSION 3.14)
project(Lab7)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(${PROJECT_NAME}_lib
    ${SRC_DIR}/NPCTypes.cpp
    ${SRC_DIR}/NPCFactory.cpp
    ${SRC_DIR}/FightRules.cpp
    ${SRC_DIR}/Observer.cpp
    ${SRC_DIR}/Editor.cpp
    ${SRC_DIR}/Game.cpp
)

target_include_directories(${PROJECT_NAME}_lib 
    PUBLIC 
    ${INCLUDES_DIR}
)

target_link_libraries(${PROJECT_NAME}_lib 
    PRIVATE 
    Threads::Threads
)

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    ${PROJECT_NAME}_lib 
    Threads::Threads
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
    ${INCLUDES_DIR}
)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)
endif()

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Binary directory: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "Build tests: ${BUILD_TESTS}")