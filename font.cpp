#include <SDL.h>

#include "font.hpp"
#include "util.hpp"

static const int FONT_WIDTH_PIXELS = 7;
static const int FONT_HEIGHT_PIXELS = 7;
static const int SCALING_FACTOR = 2;

// clang-format off
static const std::vector<bool> FONT_NUMBERS =
{
    // 0
    0,0,1,1,1,0,0,
    0,1,0,0,1,1,0,
    1,1,0,0,0,1,1,
    1,1,0,0,0,1,1,
    1,1,0,0,0,1,1,
    0,1,1,0,0,1,0,
    0,0,1,1,1,0,0,
    // 1
    0,0,1,1,0,0,0,
    0,1,1,1,0,0,0,
    0,0,1,1,0,0,0,
    0,0,1,1,0,0,0,
    0,0,1,1,0,0,0,
    0,0,1,1,0,0,0,
    1,1,1,1,1,1,1,
    // 2
    0,1,1,1,1,1,0,
    1,1,0,0,0,1,1,
    0,0,0,0,1,1,1,
    0,0,1,1,1,1,0,
    0,1,1,1,1,0,0,
    1,1,1,0,0,0,0,
    1,1,1,1,1,1,1,
    // 3
    0,1,1,1,1,1,1,
    0,0,0,0,1,1,0,
    0,0,0,1,1,0,0,
    0,0,1,1,1,1,0,
    0,0,0,0,0,1,1,
    1,1,0,0,0,1,1,
    0,1,1,1,1,1,0,
    // 4
    0,0,0,1,1,1,0,
    0,0,1,1,1,1,0,
    0,1,1,0,1,1,0,
    1,1,0,0,1,1,0,
    1,1,1,1,1,1,1,
    0,0,0,0,1,1,0,
    0,0,0,0,1,1,0,
    // 5
    1,1,1,1,1,1,0,
    1,1,0,0,0,0,0,
    1,1,1,1,1,1,0,
    0,0,0,0,0,1,1,
    0,0,0,0,0,1,1,
    1,1,0,0,0,1,1,
    0,1,1,1,1,1,0,
    // 6
    0,0,1,1,1,1,0,
    0,1,1,0,0,0,0,
    1,1,0,0,0,0,0,
    1,1,1,1,1,1,0,
    1,1,0,0,0,1,1,
    1,1,0,0,0,1,1,
    0,1,1,1,1,1,0,
    // 7
    1,1,1,1,1,1,1,
    1,1,0,0,0,1,1,
    0,0,0,0,1,1,0,
    0,0,0,1,1,0,0,
    0,0,1,1,0,0,0,
    0,0,1,1,0,0,0,
    0,0,1,1,0,0,0,
    // 8
    0,1,1,1,1,0,0,
    1,1,0,0,0,1,0,
    1,1,1,0,0,1,0,
    0,1,1,1,1,0,0,
    1,0,0,1,1,1,1,
    1,0,0,0,0,1,1,
    0,1,1,1,1,1,0,
    // 9
    0,1,1,1,1,1,0,
    1,1,0,0,0,1,1,
    1,1,0,0,0,1,1,
    0,1,1,1,1,1,1,
    0,0,0,0,0,1,1,
    0,0,0,0,1,1,0,
    0,1,1,1,1,0,0,
};
// clang-format on

static void displayFromCharset(
    SDL_Renderer* renderer, const std::vector<bool>& charset, int charIndex, int x, int y, SDL_Color color)
{
    int bitmapIndex = (FONT_HEIGHT_PIXELS * FONT_WIDTH_PIXELS) * charIndex;
    for(int lineNumber = 0; lineNumber < FONT_HEIGHT_PIXELS * SCALING_FACTOR; lineNumber += SCALING_FACTOR)
    {
        for(int pixelNumber = 0; pixelNumber < FONT_WIDTH_PIXELS * SCALING_FACTOR; pixelNumber += SCALING_FACTOR)
        {
            if(charset[bitmapIndex])
            {
                for(int lineScale = 0; lineScale < SCALING_FACTOR; lineScale++)
                    for(int pixelScale = 0; pixelScale < SCALING_FACTOR; pixelScale++)
                        SDL_RenderDrawPoint(
                            renderer, x - (FONT_WIDTH_PIXELS - pixelNumber) + pixelScale, y + lineNumber + lineScale);
            }
            ++bitmapIndex;
        }
    }
}

void displayNumber(SDL_Renderer* renderer, int x, int y, int number, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    int currentRightEdgeX = x;

    for(int remainingValue = number; remainingValue != 0; remainingValue /= 10)
    {
        displayFromCharset(renderer, FONT_NUMBERS, remainingValue % 10, currentRightEdgeX, y, color);
        currentRightEdgeX -= (FONT_WIDTH_PIXELS + 1) * SCALING_FACTOR;
    }
}