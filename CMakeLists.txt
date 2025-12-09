cmake_minimum_required(VERSION 3.22)
project(TetrisSRS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SFML 3 via vcpkg
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS include/**/*.hpp include/**/*.h)

add_executable(TetrisSRS ${SOURCES} ${HEADERS}
        include/core/Application.hpp
        include/core/Application.hpp
        include/game/GameState.hpp)
target_include_directories(TetrisSRS PRIVATE include)

target_link_libraries(TetrisSRS
  PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

# Copy runtime DLLs on Windows
if (WIN32)
  add_custom_command(TARGET TetrisSRS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:TetrisSRS> $<TARGET_FILE_DIR:TetrisSRS>
    COMMAND_EXPAND_LISTS)
endif()

# Copy resources folder next to the executable (Debug/Release)
add_custom_command(TARGET TetrisSRS POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:TetrisSRS>/resources)


# Tests (optional)
enable_testing()
add_executable(smoketest tests/smoketest.cpp
        include/core/Application.hpp
        include/game/Pieces.hpp)
target_link_libraries(smoketest PRIVATE SFML::Graphics SFML::Window SFML::System)
add_test(NAME smoketest COMMAND smoketest)


