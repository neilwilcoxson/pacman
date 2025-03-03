#pragma once

#include <array>
#include <memory>
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

    void update();
    void handleKeypress(const SDL_Keycode keyCode);
    bool gameOver();
    void handlePacmanArrival();

private:
    void drawScore();
    void drawFullBoard();
    void drawBoundary(int row, int col);

private:
    BoardLayout m_board = BASE_LAYOUT;
    Pacman m_pacman {*this};
    std::vector<std::unique_ptr<Ghost>> m_ghosts {Ghost::makeGhosts(*this)};
    PointsFruit m_fruit {*this};
    std::vector<DisplayFruit> m_displayFruits {DisplayFruit::makeDisplayFruits(*this)};

    bool m_readyDisplayed = true;
    bool m_activePlay = false;

    uint64_t readyTimerLengthTicks = 3000;

    static const inline int DEFAULT_FLASHING_GHOST_POINTS = 100;
    int m_flashingGhostPoints = DEFAULT_FLASHING_GHOST_POINTS;
    uint64_t m_flashingGhostDurationMs = 8000;

    // general scoring parameters
    int m_normalDotPoints = 10;
    int m_superDotPoints = 100;

    int m_highScore = 10'000;
    int m_score = 0;
    int m_lives = 3;
    int m_level = 1;
    int m_dotsRemaining = 1;
    int m_extraLifeThreshold = 10'000;
    int m_extraLivesIncrement = 10'000;

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
    friend class Blinky;
    friend class Pinky;
    friend class Inky;
    friend class Clyde;
    friend class DisplayFruit;
    friend class PointsFruit;
};
