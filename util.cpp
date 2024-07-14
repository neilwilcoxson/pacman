#include <SDL.h>
#include "util.hpp"

void drawFilledCircle(SDL_Renderer* renderer, const size_t xCenter, const size_t yCenter, const size_t radius, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int64_t x = 0; x < radius * 2; x++)
    {
        for (int64_t y = 0; y < radius * 2; y++)
        {
            int64_t dx = radius - x;
            int64_t dy = radius - y;
            if (dx * dx + dy * dy <= radius * radius)
            {
                SDL_RenderDrawPoint(renderer, xCenter + dx, yCenter + dy);
            }
        }
    }
}
