cmake_minimum_required(VERSION 3.20)

project(Sudoku LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# include all .h files
include_directories(${CMAKE_SOURCE_DIR}/include)

include(FetchContent)

FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.5.0
)

FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(Sudoku main.cpp
        include/Sudoku.hpp
        src/Sudoku.cpp
)

target_link_libraries(
        Sudoku
        PRIVATE
        CLI11::CLI11
        nlohmann_json::nlohmann_json
)

target_compile_options(
        Sudoku
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
