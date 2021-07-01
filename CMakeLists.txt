cmake_minimum_required(VERSION 3.19)

project(w4c-fsm)

set(CMAKE_CXX_STANDARD 20)

add_library(w4c-fsm INTERFACE)
target_include_directories(w4c-fsm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost)
if(NOT Boost_FOUND)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(w4c-fsm ${Boost_LIBRARIES})
endif()

add_executable(w4c-fsm-test test.cpp)
target_link_libraries(w4c-fsm)

