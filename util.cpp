#include <SDL.h>
#include "util.hpp"

void drawFilledCircle(
    SDL_Renderer* renderer, const int xCenter, const int yCenter, const int radius, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for(int x = 0; x < radius * 2; x++)
    {
        for(int y = 0; y < radius * 2; y++)
        {
            int dx = radius - x;
            int dy = radius - y;
            if(dx * dx + dy * dy <= radius * radius)
            {
                SDL_RenderDrawPoint(renderer, xCenter + dx, yCenter + dy);
            }
        }
    }
}
