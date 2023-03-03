add_subdirectory(dependencies/minifb)

# Link MiniFB to your project:
target_link_libraries(${PROJECT_NAME}
    minifb
)
