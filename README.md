# pacman
A clone game implemented in C++ with SDL2

<img width=400px alt="Game screenshot" src=demo/firstLevel.png>

Screenshot from August 11, 2024

## Current Features
* Accumulate points by eating dots
* Ghosts can be captured for points after eating a super dot (with time limit)
* Ghost logic attempts to match original game as described [here](https://gameinternals.com/understanding-pac-man-ghost-behavior)
* Multiple levels with distinct icons
* Wraparound when leaving the board on left or right
* Certain points thresholds award extra lives

## Possible future features
* Sound effects
* Minor graphic details
   * Rounded edges of playfield
   * Ghost eyes

## Development Notes
### clang-format enforcement
* A ```.clang-format``` file is provided in the root of the repository. Pull Requests and direct pushes to the main branch will be checked against this by GitHub actions.
* CMake targets ```format``` and ```check-format``` are provided to run on developer machines.
* ```clang-format off``` is ok to use for manually formatted arrays, strings, and similar.

## Build Directions
### General Prerequisites
1. [SDL2-devel](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.5) for your OS
    * I used SDL2-devel-2.26.5-VC for original development, but latest version of SDL2 should work
    * I'm considering adding SDL3 as a submodule here
1. Add PATH_TO_SDL\lib\x64 to your PATH
    * This is needed to run the resulting executable
    * Windows Hint: Search for environment variables in the start menu

### CMake Method (Preferred)
This method should be cross-platform
1. (If SDL2 was not installed through a package manager) Set ```SDL2_DIR``` environment variable to point to the directory containing sdl2-config.cmake
1. Create a build directory in the project root ```mkdir build```
1. Change to the build directory: ```cd build```
1. Run ```cmake ..```
1. Run platform specific compiler ```make``` or ```msbuild *.sln```
    * If msbuild is used, you may need to add it to your path

### Visual Studio (Manual Method)
Use this method if you can't or don't want to use CMake.
1. Clone this repository
1. Create a new project from existing files
     * Console application is fine
1. Select Debug or Release build from the dropdown
1. Under Local Windows Debugger > Debug Properties
    1. General > C++ Language Standard = ISO C++17 Standard (/std:c++17)
    1. C/C++ > General > Additional Include Directories > Edit
        * Add PATH_TO_SDL\include
    1. Linker > General > Additional Library Directories > Edit
        * Add PATH_TO_SDL\lib\x64
    1. Linker > General > Input > Additional Dependencies > Edit
        * Add SDL2.lib
        * Add SDL2main.lib

### General Directions for other build systems
1. Add SDL include directory to include path
1. Add SDL library directory to library path
1. Add linker option to link to SDL2 and SDL2main