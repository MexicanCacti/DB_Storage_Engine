# Packages the storage engine into a library & also creates the runner & test executables 

cmake_minimum_required(VERSION 3.14)
project(DB_STORAGE_LIBRARY LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME})

# Include all DB .cpp files
target_sources(${PROJECT_NAME}
    PRIVATE
        engine/field/field.cpp
        engine/schema/schema.cpp
        engine/tuple/tuple.cpp
)

# Include all DB Directories
target_include_directories(${PROJECT_NAME}
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/field
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/schema
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/tuple

    # Include all DB directories that contain the .hpp files
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/field>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/schema>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/tuple>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_subdirectory(runner)
add_subdirectory(tests)

if(DEFINED CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(
        STATUS
        "CMAKE_INSTALL_PREFIX is not set\n"
        "Default value: ${CMAKE_INSTALL_PREFIX}\n"
        "Will set it to ${CMAKE_SOURCE_DIR}/libs"
    )
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_SOURCE_DIR}/libs"
        CACHE PATH "Where the library will be installed to" FORCE
    )
else()
    message(
        STATUS
        "CMAKE_INSTALL_PREFIX was already set\n"
        "Current value: ${CMAKE_INSTALL_PREFIX}"
    )
endif()

# Every header used by the DB Storage Engine should be here. Basically makes public to other projects using the library
set(public_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/field/field.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/schema/schema.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/tuple/tuple.hpp
)

set_target_properties(DB_STORAGE_LIBRARY PROPERTIES PUBLIC_HEADER "${public_headers}")

# To preserve folder structure
include(GNUInstallDirs)

foreach(header ${public_headers})
    file(RELATIVE_PATH header_file_path "${CMAKE_CURRENT_SOURCE_DIR}/" "${header}")
    get_filename_component(header_directory_path "${header_file_path}" DIRECTORY)
    install(
        FILES ${header}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${header_directory_path}"
    )
endforeach()

# Install instructions
install(TARGETS ${PROJECT_NAME}
    EXPORT "${PROJECT_NAME}Targets"
    # except for public headers, as we want them to be inside a library folder
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME} # lib/SomeLibrary
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # lib
)

# generate and install export file
install(EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    NAMESPACE ${namespace}::
    DESTINATION cmake
)

include(CMakePackageConfigHelpers)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION "1.0"
    COMPATIBILITY AnyNewerVersion
)
# create config file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION cmake
)
# install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION cmake
)