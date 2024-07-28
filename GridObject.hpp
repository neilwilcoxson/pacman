#pragma once

#include <SDL.h>

#include "util.hpp"

// forward declaration
class GameState;

struct GridPosition
{
    int row;
    int col;
};

class GridObject
{
public:
    GridObject() = delete;
    GridObject(GridObject&) = delete;
    GridObject(GridObject&&) = default;
    GridObject(GameState& gameState, int row, int col);
    virtual void update() = 0;
    virtual void reset() = 0;
    GridPosition getPosition() const
    {
        return {m_row, m_col};
    }
    bool hasSamePositionAs(const GridObject& otherObject) const;
    void relocate(int row, int col);

protected:
    int m_row;
    int m_col;
    int m_xPixelOffset = 0; // offset from center within the column
    int m_yPixelOffset = 0; // offset from center within the row
    std::string m_name;
    GameState& m_gameState;
};

class Mover : public GridObject
{
public:
    Mover() = delete;
    Mover(Mover&) = delete;
    Mover(Mover&&) = default;
    Mover(GameState& gameState, int startRow, int startCol, Direction startFacing);
    void changeDirection(Direction newDirection);

protected:
    virtual void handleArrival() {};
    virtual void handleWall() = 0;
    virtual void handleMovement();
    bool directionValid(const Direction newDirection) const;
    bool directionIsCloser(const Direction newDirection, const Mover& otherMover) const;

protected:
    Direction m_facingDirection = Direction::LEFT;
    Direction m_pendingDirection = Direction::LEFT;
    uint64_t m_lastDrawnTicks = 0;
    int m_velocity = 100; // pixels per second (might need to change based on resizable window)
};

class Pacman : public Mover
{
public:
    Pacman() = delete;
    Pacman(Pacman&) = delete;
    Pacman(Pacman&&) = default;
    Pacman(GameState& gameState);
    void update() override;
    void reset() override;

protected:
    void handleArrival() override;
    void handleWall() override;

private:
    static inline const int PACMAN_START_ROW = 23;
    static inline const int PACMAN_START_COL = 14;
    static inline const Direction PACMAN_START_DIRECTION = Direction::LEFT;
    static inline const SDL_Color COLOR = COLOR_YELLOW;
    static inline const int RADIUS = 14;

    int m_mouthPixels = 0;
    int m_mouthIncrement = 1;
};

class Ghost : public Mover
{
public:
    static std::vector<Ghost> makeGhosts(GameState& gameState);

    Ghost() = delete;
    Ghost(Ghost&) = delete;
    Ghost(Ghost&&) = default;
    Ghost(
        GameState& gameState,
        int startRow,
        int startCol,
        Direction startFacing,
        const SDL_Color& color,
        const std::string& name);
    void update() override;
    void reset() override;
    void handleSuperDot();

protected:
    void handleArrival() override;
    void handleWall() override;

public:
    bool m_inBox = true;
    bool m_isFlashing = false;

private:
    static inline const int NUM_GHOSTS = 4;
    static inline const int GHOST_START_ROW = 15;
    static inline const int GHOST_START_COL = 13;
    static inline const Direction GHOST_START_DIRECTION = Direction::LEFT;
    static inline const SDL_Color FLASH_COLOR[2] = {COLOR_WHITE, COLOR_BLUE};

    static inline int nextIndex = 0;
    const int m_index;

    SDL_Color m_color;

    int m_flashColorIndex = 0;
    IntervalDeadlineTimer m_flashColorTimer {1000, true, [this]() { m_flashColorIndex = 1 - m_flashColorIndex; }};

    int m_awayFromPacmanDirectionInterval = 10;
    int m_numMovesTowardPacman = 0;
};

class DisplayFruit : public GridObject
{
public:
    static std::vector<DisplayFruit> makeDisplayFruits(GameState& gameState);

    DisplayFruit(GameState& gameState, int index);
    DisplayFruit() = delete;
    DisplayFruit(DisplayFruit&) = delete;
    DisplayFruit(DisplayFruit&&) = default;
    virtual void update() override;
    virtual void reset() override {};

private:
    static inline const int FRUIT_DISPLAY_ROW = 31;
    static inline const int FRUIT_DISPLAY_START_COL = 27;
};

class PointsFruit : public DisplayFruit
{
public:
    PointsFruit() = delete;
    PointsFruit(PointsFruit&) = delete;
    PointsFruit(PointsFruit&&) = default;
    PointsFruit(GameState& gameState);
    virtual void update() override;
    void reset() override;
    void activate();
    inline bool isActive()
    {
        return m_available;
    }

private:
    static inline const int FRUIT_SPAWN_ROW = 18;
    static inline const int FRUIT_SPAWN_COL = 14;
    static inline const int FRUIT_DURATION_TICKS = 8000;

    // clang-format off
    bool m_available = false;
    IntervalDeadlineTimer m_availabilityTimer {
        FRUIT_DURATION_TICKS,
        false,
        [this]() {
            m_available = false;
        }
    };
    // clang-format on
};
