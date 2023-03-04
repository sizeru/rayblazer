cmake_minimum_required(VERSION 2.8.0)
project(rayblazer)

add_subdirectory(dependencies/minifb)


set (SOURCES src/main.cpp src/vector.hpp)
add_executable(${PROJECT_NAME} ${SOURCES})

# Link MiniFB to your project:
target_link_libraries(${PROJECT_NAME} minifb)