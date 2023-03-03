cmake_minimum_required(VERSION 2.8.0)
project(rayblazer)

add_subdirectory(dependencies/minifb)

add_executable(${PROJECT_NAME} src/main.cpp)

# Link MiniFB to your project:
target_link_libraries(${PROJECT_NAME} minifb)