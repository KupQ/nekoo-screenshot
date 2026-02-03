cmake_minimum_required(VERSION 3.15)
project(nekoo-screenshot)

set(CMAKE_CXX_STANDARD 17)

add_executable(nekoo-screenshot main.cpp)

target_link_libraries(nekoo-screenshot
    gdiplus
    winhttp
    user32
)

set_target_properties(nekoo-screenshot PROPERTIES
    WIN32_EXECUTABLE FALSE
)
