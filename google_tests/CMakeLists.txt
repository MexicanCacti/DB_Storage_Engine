cmake_minimum_required(VERSION 3.10)
project(DB_STORAGE_ENGINE_GOOGLE_TESTS)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable( basic_types_tests
  basic_types_tests.cpp
)

add_executable( helper_tests
  helper_tests.cpp
)

add_executable( basic_types_encode_decode_tests
  basic_types_encode_decode_tests.cpp
)

add_executable( page_tests
  page_tests.cpp
)

add_executable( schema_tests
  schema_tests.cpp
)

add_executable( tuple_tests
  tuple_tests.cpp
)

add_executable( table_header_tests
  table_header_tests.cpp
)

add_executable( data_page_header_tests
  data_page_header_tests.cpp
)

add_executable( bitmap_tests
  bitmap_tests.cpp
)

add_executable( data_page_tests
  data_page_tests.cpp
)

add_executable( file_manager_tests
  file_manager_tests.cpp
)

add_executable(all_tests
    basic_types_tests.cpp
    helper_tests.cpp
    basic_types_encode_decode_tests.cpp
    page_tests.cpp
    schema_tests.cpp
    tuple_tests.cpp
    table_header_tests.cpp
    data_page_header_tests.cpp
    data_page_tests.cpp
    bitmap_tests.cpp
    file_manager_tests.cpp
)

target_link_libraries( basic_types_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( helper_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( basic_types_encode_decode_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( page_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( schema_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( tuple_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( table_header_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( data_page_header_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( bitmap_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( data_page_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries( file_manager_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

target_link_libraries(all_tests
    PRIVATE
        DB_STORAGE_LIBRARY
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(basic_types_tests)
gtest_discover_tests(helper_tests)
gtest_discover_tests(basic_types_encode_decode_tests)
gtest_discover_tests(page_tests)
gtest_discover_tests(schema_tests)
gtest_discover_tests(tuple_tests)
gtest_discover_tests(table_header_tests)
gtest_discover_tests(data_page_header_tests)
gtest_discover_tests(bitmap_tests)
gtest_discover_tests(data_page_tests)
gtest_discover_tests(file_manager_tests)
gtest_discover_tests(all_tests)

set_target_properties(
    basic_types_tests helper_tests basic_types_encode_decode_tests 
    page_tests schema_tests tuple_tests table_header_tests 
    data_page_header_tests bitmap_tests data_page_tests 
    file_manager_tests
    all_tests
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

