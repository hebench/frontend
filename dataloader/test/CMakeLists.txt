cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_executable(test_csv "test_csv.cpp")
target_link_libraries(test_csv PRIVATE dataloader)
add_test(NAME test_csv COMMAND test_csv)

file(COPY "data" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
