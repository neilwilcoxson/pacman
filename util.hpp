#pragma once

#include <stdio.h>
#include <functional>
#include <string>
#include <vector>

// forward declaration
struct SDL_Renderer;

#define LOG_LEVEL_TRACE (6)
#define LOG_LEVEL_DEBUG (5)
#define LOG_LEVEL_INFO (4)
#define LOG_LEVEL_WARN (3)
#define LOG_LEVEL_ERROR (2)
#define LOG_LEVEL_ASSERT (1)

#define activeLevel LOG_LEVEL_INFO

// GNU C++ doesn't handle empty __VA_ARGS__ the same as MSVC
#ifdef __GNUG__
#define LOG_AT_LEVEL(level, format, ...) \
    if(activeLevel >= level)             \
    printf("[%s] %s:%d: " format "\n", #level, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_TRACE(format, ...) LOG_AT_LEVEL(LOG_LEVEL_TRACE, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) LOG_AT_LEVEL(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LOG_AT_LEVEL(LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG_AT_LEVEL(LOG_LEVEL_WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_AT_LEVEL(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define LOG_ASSERT(condition, format, ...)                                           \
    if(!(condition))                                                                 \
    {                                                                                \
        LOG_AT_LEVEL(LOG_LEVEL_ASSERT, #condition " fails, " format, ##__VA_ARGS__); \
        exit(EXIT_FAILURE);                                                          \
    }
#else
#define LOG_AT_LEVEL(level, format, ...) \
    if(activeLevel >= level)             \
    printf("[%s] %s:%d: " format "\n", #level, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_TRACE(format, ...) LOG_AT_LEVEL(LOG_LEVEL_TRACE, format, __VA_ARGS__)
#define LOG_DEBUG(format, ...) LOG_AT_LEVEL(LOG_LEVEL_DEBUG, format, __VA_ARGS__)
#define LOG_INFO(format, ...) LOG_AT_LEVEL(LOG_LEVEL_INFO, format, __VA_ARGS__)
#define LOG_WARN(format, ...) LOG_AT_LEVEL(LOG_LEVEL_WARN, format, __VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_AT_LEVEL(LOG_LEVEL_ERROR, format, __VA_ARGS__)
#define LOG_ASSERT(condition, format, ...)                                         \
    if(!(condition))                                                               \
    {                                                                              \
        LOG_AT_LEVEL(LOG_LEVEL_ASSERT, #condition " fails, " format, __VA_ARGS__); \
        exit(EXIT_FAILURE);                                                        \
    }
#endif

const int X_INCREMENT[] = {0, 0, -1, 1, 0};
const int Y_INCREMENT[] = {-1, 1, 0, 0, 0};

typedef std::vector<std::string> BoardLayout;
const char DOT = '.';
const char SUPER_DOT = '*';
const char WRAP = 'w';
const char VERTICAL_WALL = '|';
const char HORIZONTAL_WALL = '-';
const char UL_CORNER = 'T';
const char UR_CORNER = '7';
const char LL_CORNER = 'L';
const char LR_CONRER = 'J';

// clang-format off
const BoardLayout BASE_LAYOUT =
{
    "                              ",
    " T--------------------------7 ",
    " |............||............| ",
    " |.T--7.T---7.||.T---7.T--7.| ",
    " |*|  |.|   |.||.|   |.|  |*| ",
    " |.L--J.L---J.LJ.L---J.L--J.| ",
    " |..........................| ",
    " |.T--7.T7.T------7.T7.T--7.| ",
    " |.L--J.||.L--7T--J.||.L--J.| ",
    " |......||....||....||......| ",
    " L----7.|T--- LJ ---7|.T----J ",
    "      |.||          ||.|      ",
    "      |.|| T------7 ||.|      ",
    "      |.|| |      | ||.|      ",
    " -----J.LJ |      | LJ.L----- ",
    " w     .   |      |   .     w ",
    " -----7.T7 |      | T7.T----- ",
    "      |.|| L------J ||.|      ",
    "      |.||          ||.|      ",
    " T----J.LJ.-------- LJ.L----7 ",
    " |............T7............| ",
    " |.T--7.T---7.||.T---7.T--7.| ",
    " |.L-7|.L---J.LJ.L---J.|T-J.| ",
    " |...||....... ........||...| ",
    " |-7.||.T7.T------7.T7.||.T-| ",
    " |-J.LJ.||.L--7T--J.||.LJ.L-| ",
    " |......||....||....||......| ",
    " |.T----JL--7.||.T--JL----7.| ",
    " |.L--------J.LJ.L--------J.| ",
    " |..........................| ",
    " L--------------------------J ",
    "                              "
};
// clang-format on

const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 960;
const int TILE_WIDTH = SCREEN_WIDTH / (int)BASE_LAYOUT[0].length();
const int TILE_HEIGHT = SCREEN_HEIGHT / (int)BASE_LAYOUT.size();

const SDL_Color COLOR_RED = {0xff, 0x00, 0x00, 0xff};
const SDL_Color COLOR_GREEN = {0x00, 0xff, 0x00, 0xff};
const SDL_Color COLOR_BLUE = {0x00, 0x00, 0xff, 0xff};
const SDL_Color COLOR_YELLOW = {0xff, 0xff, 0x00, 0xff};
const SDL_Color COLOR_WHITE = {0xff, 0xff, 0xff, 0xff};
const SDL_Color COLOR_PINK = {0xff, 0xb6, 0xc1, 0xff};
const SDL_Color COLOR_TURQUOISE = {0x30, 0xd5, 0xc8, 0xff};
const SDL_Color COLOR_ORANGE = {0xff, 0x8c, 0x00, 0xff};
const SDL_Color COLOR_BROWN = {0xa0, 0x52, 0x2d, 0xff};
const SDL_Color COLOR_TAN = {0xd2, 0xb4, 0x8c, 0xff};
const SDL_Color COLOR_BABY_BLUE = {0x89, 0xcf, 0xc0, 0xff};

enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    MAX
};

const char* const DIRECTION_AS_STRING[] = {"UP", "DOWN", "LEFT", "RIGHT", "MAX"};

#define X_CENTER(col) ((col)*TILE_WIDTH + TILE_WIDTH / 2)
#define Y_CENTER(row) ((row)*TILE_HEIGHT + TILE_HEIGHT / 2)

inline bool isBoundary(char c)
{
    switch(c)
    {
    case VERTICAL_WALL:
    case HORIZONTAL_WALL:
    case UL_CORNER:
    case UR_CORNER:
    case LL_CORNER:
    case LR_CONRER:
        return true;
    default:
        return false;
    }
}
void drawFilledCircle(
    SDL_Renderer* renderer, const int xCenter, const int yCenter, const int radius, const SDL_Color& color);
