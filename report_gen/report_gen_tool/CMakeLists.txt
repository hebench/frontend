cmake_minimum_required(VERSION 2.9)
project(reportgen_tool)

set(reportgen_tool_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(reportgen_tool ${reportgen_tool_SOURCES} ${reportgen_tool_HEADERS})

target_include_directories(reportgen_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(reportgen_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(reportgen_tool PRIVATE hebench_reportgen_lib)
target_link_libraries(reportgen_tool PRIVATE hebench_reportgen)
target_compile_options(reportgen_tool PRIVATE -Wall -Wextra)

install(TARGETS reportgen_tool DESTINATION bin)
