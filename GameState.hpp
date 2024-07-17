#pragma once

#include <optional>
#include <string>
#include <vector>

#include "GridObject.hpp"
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
    void drawScore();
    void drawFullBoard();
private:
    BoardLayout m_board = BASE_LAYOUT;
    Pacman m_pacman { *this };
    std::vector<Ghost> m_ghosts = Ghost::makeGhosts(*this);
    Fruit m_fruit{ *this };

    uint64_t m_ghostSpwanIntervalTicks = 2000;
    uint64_t m_nextGhostTicks = 0;

    static const inline int DEFAULT_FLASHING_GHOST_POINTS = 100;
    int m_flashingGhostPoints = DEFAULT_FLASHING_GHOST_POINTS;
    int m_flashingGhostDurationMs = 8000;
    std::optional<uint64_t> m_flashingGhostDeadline;

    // general scoring parameters
    int m_normalDotPoints = 10;
    int m_superDotPoints = 100;

    uint64_t m_score = 0;
    int m_lives = 3;
    int m_level = 1;

    // tracking for fruit
    int m_dotsEaten = 0;
    int m_fruitThreshold = 70;
    int m_fruitThresholdIncrement = 100;
    int m_fruitPoints = 100;
    int m_fruitPointsMultiplier = 2;

    SDL_Renderer* m_renderer;

    friend class Mover;
    friend class Pacman;
    friend class Ghost;
    friend class Fruit;
};
