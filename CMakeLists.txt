cmake_minimum_required(VERSION 3.0)
project(pacman)

# Linux:   apt install libsdl2-dev
#          (Note: check that the version is new enough if there are errors)
# Mac:     brew install sdl2
# Windows: set environment variable SDL2_DIR,
#          points to the directory containing sdl2-config.cmake
find_package(SDL2 2.0.18 REQUIRED)

add_executable(pacman pacman.cpp)
target_compile_features(pacman PUBLIC cxx_std_17)
target_include_directories(pacman PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(pacman PRIVATE ${SDL2_LIBRARIES})
target_sources(pacman PRIVATE GameState.cpp GridObject.cpp util.cpp)

add_custom_target(format
    COMMAND clang-format -i *.cpp *.hpp
    COMMENT "Running clang-format")
add_custom_target(check-format
    COMMAND clang-format --dry-run *.cpp *.hpp
    COMMENT "Running clang-format --dry-run")