#pragma once

#include <string>
#include <vector>

#include "Mover.hpp"
#include "util.hpp"

// forward declaration
struct SDL_Renderer;

class GameState
{
public:
    GameState(SDL_Renderer* renderer);
    void draw();
    void handleKeypress(const SDL_KeyCode keyCode);
private:
    BoardLayout m_board = BASE_LAYOUT;
    Pacman m_pacman{ 23, 14, Direction::LEFT };
    std::vector<Ghost> m_ghosts;

    uint64_t m_ghostSpwanIntervalTicks = 2000;
    uint64_t m_nextGhostTicks = 0;

    SDL_Renderer* m_renderer;
};

void drawFullBoard(SDL_Renderer* renderer, BoardLayout& board);