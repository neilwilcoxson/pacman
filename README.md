# pacman
A clone game implemented in C++ with SDL2

## Build Directions
### General Prerequisites
1. [SDL2-devel](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.5) for your OS
    * I used SDL2-devel-2.26.5-VC for original development, but latest version of SDL2 should work
    * I'm considering adding SDL3 as a submodule here
1. Add PATH_TO_SDL\lib\x64 to your PATH
    * Hint: Search for environment variables in the start menu

### General Overview Compiler Setup
1. Add SDL include directory to include path
1. Add SDL library directory to library path
1. Add linker option to link to SDL2 and SDL2main

### Visual Studio
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