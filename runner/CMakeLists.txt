cmake_minimum_required(VERSION 3.14)
project(DB_RUNNER LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(DB_RUNNER
    main.cpp
)

target_link_libraries(DB_RUNNER
    PRIVATE
        DB_STORAGE_LIBRARY
)

# creates the exe in runner
set_target_properties(DB_RUNNER PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)