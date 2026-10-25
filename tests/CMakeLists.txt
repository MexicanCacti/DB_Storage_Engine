cmake_minimum_required(VERSION 3.10)
project(DB_STORAGE_ENGINE_TESTS)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable( 4_6_tests
  4_6_tests.cpp
)

add_executable(all_user_tests
    4_6_tests.cpp
)

target_link_libraries(4_6_tests
    PRIVATE
        DB_STORAGE_LIBRARY
)

target_link_libraries(all_user_tests
    PRIVATE
        DB_STORAGE_LIBRARY
)

set_target_properties(
    4_6_tests all_user_tests
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

