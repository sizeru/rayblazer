cmake_minimum_required(VERSION 2.8.0)
project(rayblazer)

add_subdirectory(dependencies/minifb)
add_subdirectory(dependencies/glm)
find_package(Threads)

add_compile_options(
    # -mavx512f
)

set (SOURCES src/main.cpp src/primitives.hpp src/primitives.cpp src/parser.hpp src/parser.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

# Link MiniFB to your project:
target_link_libraries(${PROJECT_NAME} minifb)
target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})