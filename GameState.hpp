#pragma once

#include <optional>
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
    bool gameOver();
    void handlePacmanArrival();
private:
    BoardLayout m_board = BASE_LAYOUT;
    Pacman m_pacman { *this };
    std::vector<Ghost> m_ghosts = Ghost::makeGhosts(*this);

    uint64_t m_ghostSpwanIntervalTicks = 2000;
    uint64_t m_nextGhostTicks = 0;

    static const inline int DEFAULT_FLASHING_GHOST_POINTS = 100;
    int m_flashingGhostPoints = DEFAULT_FLASHING_GHOST_POINTS;
    int m_flashingGhostDurationMs = 8000;
    std::optional<uint64_t> m_flashingGhostDeadline;

    int m_normalDotPoints = 10;
    int m_superDotPoints = 100;

    uint64_t m_score = 0;
    int m_lives = 3;

    SDL_Renderer* m_renderer;

    friend class Mover;
    friend class Pacman;
    friend class Ghost;
};

void drawFullBoard(SDL_Renderer* renderer, BoardLayout& board);