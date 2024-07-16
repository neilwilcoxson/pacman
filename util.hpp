#pragma once

#include <stdio.h>
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

#define LOG_AT_LEVEL(level, format, ...) if(activeLevel >= level) printf("[%s] %s:%d: " format "\n", #level, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_TRACE(format, ...) LOG_AT_LEVEL(LOG_LEVEL_TRACE, format, __VA_ARGS__)
#define LOG_DEBUG(format, ...) LOG_AT_LEVEL(LOG_LEVEL_DEBUG, format, __VA_ARGS__)
#define LOG_INFO(format, ...) LOG_AT_LEVEL(LOG_LEVEL_INFO, format, __VA_ARGS__)
#define LOG_WARN(format, ...) LOG_AT_LEVEL(LOG_LEVEL_WARN, format, __VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_AT_LEVEL(LOG_LEVEL_ERROR, format, __VA_ARGS__)
#define LOG_ASSERT(condition, format, ...) if(!(condition)) { LOG_AT_LEVEL(LOG_LEVEL_ASSERT, #condition " fails, " format, __VA_ARGS__); exit(EXIT_FAILURE); }

typedef std::vector<std::string> BoardLayout;
const char BOUNDARY = 'x';
const char DOT = '.';
const char SUPER_DOT = '*';
const char WRAP = 'w';
const BoardLayout BASE_LAYOUT =
{
    "                              ",
    " xxxxxxxxxxxxxxxxxxxxxxxxxxxx ",
    " x............xx............x ",
    " x.xxxx.xxxxx.xx.xxxxx.xxxx.x ",
    " x*x  x.x   x.xx.x   x.x  x*x ",
    " x.xxxx.xxxxx.xx.xxxxx.xxxx.x ",
    " x..........................x ",
    " x.xxxx.xx.xxxxxxxx.xx.xxxx.x ",
    " x.xxxx.xx.xxxxxxxx.xx.xxxx.x ",
    " x......xx....xx....xx......x ",
    " xxxxxx.xxxxx xx xxxxx.xxxxxx ",
    "      x.xx          xx.x      ",
    "      x.xx xxxxxxxx xx.x      ",
    "      x.xx x      x xx.x      ",
    " xxxxxx.xx x      x xx.xxxxxx ",
    " w     .   x      x   .     w ",
    " xxxxxx.xx x      x xx.xxxxxx ",
    "      x.xx xxxxxxxx xx.x      ",
    "      x.xx          xx.x      ",
    " xxxxxx.xx.xxxxxxxx xx.xxxxxx ",
    " x............xx............x ",
    " x.xxxx.xxxxx.xx.xxxxx.xxxx.x ",
    " x.xxxx.xxxxx.xx.xxxxx.xxxx.x ",
    " x...xx....... ........xx...x ",
    " xxx.xx.xx.xxxxxxxx.xx.xx.xxx ",
    " xxx.xx.xx.xxxxxxxx.xx.xx.xxx ",
    " x......xx....xx....xx......x ",
    " x.xxxxxxxxxx.xx.xxxxxxxxxx.x ",
    " x.xxxxxxxxxx.xx.xxxxxxxxxx.x ",
    " x..........................x ",
    " xxxxxxxxxxxxxxxxxxxxxxxxxxxx ",
    "                               "
};
const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 960;
const int TILE_WIDTH = SCREEN_WIDTH / (int)BASE_LAYOUT[0].length();
const int TILE_HEIGHT = SCREEN_HEIGHT / (int)BASE_LAYOUT.size();

const SDL_Color COLOR_RED = { 0xff, 0x00, 0x00, 0xff };
const SDL_Color COLOR_GREEN = { 0x00, 0xff, 0x00, 0xff };
const SDL_Color COLOR_BLUE = { 0x00, 0x00, 0xff, 0xff };
const SDL_Color COLOR_YELLOW = { 0xff, 0xff, 0x00, 0xff };
const SDL_Color COLOR_WHITE = { 0xff, 0xff, 0xff, 0xff };

enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    MAX
};

const char* const DIRECTION_AS_STRING[] =
{
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "MAX"
};

#define X_CENTER(col) ((col) * TILE_WIDTH + TILE_WIDTH / 2)
#define Y_CENTER(row) ((row) * TILE_HEIGHT + TILE_HEIGHT / 2)

void drawFilledCircle(SDL_Renderer* renderer, const size_t xCenter, const size_t yCenter, const size_t radius, const SDL_Color& color);