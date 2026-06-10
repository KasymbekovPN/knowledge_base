cmake_minimum_required(VERSION 3.20)

project(DemoProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app
    demo.cpp
    value.cpp
)