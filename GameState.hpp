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
    Pacman m_pacman { *this };
    std::vector<Ghost> m_ghosts { Ghost::makeGhosts(*this) };
    PointsFruit m_fruit{ *this };
    std::vector<DisplayFruit> m_displayFruits { DisplayFruit::makeDisplayFruits(*this) };

    // clang-format off
    uint64_t m_ghostSpawnIntervalTicks = 2000;
    IntervalDeadlineTimer m_ghostSpawnTimer {
        m_ghostSpawnIntervalTicks,
        true,
        [this]() {
            LOG_INFO("Spawning ghost");
            for (auto& ghost : m_ghosts)
            {
                if (ghost.m_inBox)
                {
                    const int GHOST_SPAWN_ROW = 11;
                    const int GHOST_SPAWN_COL = 15;
                    ghost.relocate(GHOST_SPAWN_ROW, GHOST_SPAWN_COL);
                    ghost.m_inBox = false;
                    break;
                }
            }
        }
    };

    static const inline int DEFAULT_FLASHING_GHOST_POINTS = 100;
    int m_flashingGhostPoints = DEFAULT_FLASHING_GHOST_POINTS;
    uint64_t m_flashingGhostDurationMs = 8000;
    IntervalDeadlineTimer m_flashingGhostTimer {
        m_flashingGhostDurationMs,
        false,
        [this]() {
            m_flashingGhostPoints = DEFAULT_FLASHING_GHOST_POINTS;
            for(auto& ghost : m_ghosts) ghost.m_isFlashing = false;
        }
    };
    // clang-format on

    // general scoring parameters
    int m_normalDotPoints = 10;
    int m_superDotPoints = 100;

    uint64_t m_score = 0;
    int m_lives = 3;
    int m_level = 1;
    int m_dotsRemaining = 1;

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
    friend class DisplayFruit;
    friend class PointsFruit;
};
