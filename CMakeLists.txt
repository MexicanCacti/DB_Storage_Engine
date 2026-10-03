cmake_minimum_required(VERSION 3.10)
project(DB_STORAGE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
        ./src/*.cpp
        ./src/*.hpp
        ./src/*/*.cpp
        ./src/*/*.hpp
)

add_executable(DB_STORAGE
    ${SOURCES}
)
