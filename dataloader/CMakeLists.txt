cmake_minimum_required(VERSION 3.0.0)
project(dataloader VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(dataloader
    "src/csv.cpp"
    )
target_include_directories(dataloader PUBLIC "include")

add_subdirectory("test" "test")
