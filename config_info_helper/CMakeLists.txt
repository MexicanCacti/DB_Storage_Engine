cmake_minimum_required(VERSION 3.14)

project(CONFIG_INFO_HELPER LANGUAGES CXX)

include(CMakePrintHelpers)

# System information
foreach(key IN ITEMS
    NUMBER_OF_LOGICAL_CORES
    NUMBER_OF_PHYSICAL_CORES
    IS_64BIT
    HAS_SSE2
    OS_NAME
    OS_RELEASE
)
    cmake_host_system_information(
        RESULT _${key}
        QUERY ${key}
    )

    cmake_print_variables(_${key})
endforeach()

# Compiler information
cmake_print_variables(
    CMAKE_CXX_COMPILER
    CMAKE_CXX_COMPILER_ID
    CMAKE_CXX_COMPILER_VERSION
    CMAKE_CXX_COMPILER_FRONTEND_VARIANT
)

add_executable(config_info config_info.cpp)

target_include_directories(config_info
    PRIVATE
        ${PROJECT_BINARY_DIR}
)

set_target_properties(
    config_info
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

configure_file(config-template.h config.h @ONLY)