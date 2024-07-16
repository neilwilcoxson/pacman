#include <SDL.h>    
#include <vector>

#include "GameState.hpp"

int main(int argc, char** argv)
{
    LOG_ASSERT(SDL_Init(SDL_INIT_EVERYTHING) == 0, "SDL init error: %s", SDL_GetError());

    SDL_Window* window = SDL_CreateWindow("Pacman", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    LOG_ASSERT(window != nullptr, "SDL create window error: %s", SDL_GetError());

    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    LOG_ASSERT(renderer != nullptr, "Error creating Renderer: %s", SDL_GetError());

    GameState gameState(renderer);

    SDL_Event e;
    while (!gameState.gameOver())
    {
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                goto quit;
            case SDL_KEYDOWN:
                gameState.handleKeypress((SDL_KeyCode)e.key.keysym.sym);
                break;
            default:
                break;
            }
        }

        gameState.draw();
    }

quit:
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}