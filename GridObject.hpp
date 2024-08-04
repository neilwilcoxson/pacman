#pragma once

#include <memory>
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
    Direction getDirection() const
    {
        return m_facingDirection;
    }

protected:
    virtual void handleArrival() {};
    virtual void handleWall() = 0;
    virtual void handleMovement();
    bool directionValid(const Direction newDirection) const;
    bool directionIsCloser(const Direction newDirection, const GridPosition& otherPosition) const;

protected:
    Direction m_facingDirection = Direction::LEFT;
    Direction m_pendingDirection = Direction::LEFT;
    uint64_t m_lastDrawnTicks = 0;
    int m_velocity = 100; // pixels per second (might need to change based on resizable window)
};

class Pacman : public Mover
{
public:
    static inline const int RADIUS = 14;
    static void drawPacman(
        SDL_Renderer* renderer,
        const int xCenter,
        const int yCenter,
        const Direction facingDirection,
        const int mouthPixels);

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

    int m_mouthPixels = 0;
    int m_mouthIncrement = 1;
};

class Ghost : public Mover
{
public:
    static std::vector<std::unique_ptr<Ghost>> makeGhosts(GameState& gameState);

    enum class ChaseMode
    {
        CHASE,
        SCATTER,
        FRIGHTENED
    };

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
    void resetChaseState();

protected:
    void handleArrival() override;
    void handleWall() override;
    virtual void calculateTargetLocation() = 0;
    virtual bool shouldLeaveBox() = 0;

private:
    void setChaseMode(const ChaseMode chaseMode);
    void advanceChaseState();

public:
    bool m_inBox = true;
    bool m_isFlashing = false;

protected:
    static inline const int GHOST_START_ROW = 15;
    static inline const int GHOST_START_COL = 13;
    static inline const Direction GHOST_START_DIRECTION = Direction::LEFT;

    ChaseMode m_chaseMode = ChaseMode::SCATTER;
    GridPosition m_targetLocation;
    GridPosition m_defaultTargetLocation;

private:
    static inline const int NUM_GHOSTS = 4;
    static inline const int GHOST_SPAWN_ROW = 11;
    static inline const int GHOST_SPAWN_COL = 15;
    static inline const SDL_Color FLASH_COLOR[2] = {COLOR_WHITE, COLOR_BLUE};

    static inline int nextIndex = 0;
    const int m_index;

    SDL_Color m_color;
    int m_flashColorIndex = 0;
    size_t m_flashingGhostTimerKey;

    enum class ChaseState
    {
        INITIAL_STATE,
        SCATTER_0, // 7 seconds
        CHASE_0,   // 20 seconds
        SCATTER_1, // 7 seconds
        CHASE_1,   // 20 seconds
        SCATTER_2, // 5 seconds
        CHASE_2,   // 20 seconds
        SCATTER_3, // 5 seconds
        CHASE_PERMANENT
    };

    struct ChaseStateSettings
    {
        Ghost::ChaseMode chaseMode;
        uint64_t durationMs;
    };

    std::array<ChaseStateSettings, (size_t)ChaseState::CHASE_PERMANENT + 1> m_chaseSettings {
        {{Ghost::ChaseMode::SCATTER, 0},
         {Ghost::ChaseMode::SCATTER, 7000},
         {Ghost::ChaseMode::CHASE, 20000},
         {Ghost::ChaseMode::SCATTER, 7000},
         {Ghost::ChaseMode::CHASE, 20000},
         {Ghost::ChaseMode::SCATTER, 5000},
         {Ghost::ChaseMode::CHASE, 20000},
         {Ghost::ChaseMode::SCATTER, 5000},
         {Ghost::ChaseMode::CHASE, 0}}};

    size_t m_chaseStateTimerKey;
    ChaseState m_chaseState = ChaseState::INITIAL_STATE;
};

class Blinky : public Ghost
{
public:
    Blinky(GameState& gameState)
    : Ghost(gameState, GHOST_START_ROW, GHOST_START_COL + 0, GHOST_START_DIRECTION, COLOR_RED, "Blinky")
    {
        m_defaultTargetLocation = {0, 0};
    }

private:
    void calculateTargetLocation() override;
    bool shouldLeaveBox() override;
};

class Pinky : public Ghost
{
public:
    Pinky(GameState& gameState)
    : Ghost(gameState, GHOST_START_ROW, GHOST_START_COL + 1, GHOST_START_DIRECTION, COLOR_PINK, "Pinky")
    {
        m_defaultTargetLocation = {0, 30};
    }

private:
    void calculateTargetLocation() override;
    bool shouldLeaveBox() override;
};

class Inky : public Ghost
{
public:
    Inky(GameState& gameState)
    : Ghost(gameState, GHOST_START_ROW, GHOST_START_COL + 2, GHOST_START_DIRECTION, COLOR_TURQUOISE, "Inky")
    {
        m_defaultTargetLocation = {32, 0};
    }

private:
    void calculateTargetLocation() override;
    bool shouldLeaveBox() override;
};

class Clyde : public Ghost
{
public:
    Clyde(GameState& gameState)
    : Ghost(gameState, GHOST_START_ROW, GHOST_START_COL + 3, GHOST_START_DIRECTION, COLOR_ORANGE, "Clyde")
    {
        m_defaultTargetLocation = {32, 32};
    }

private:
    void calculateTargetLocation() override;
    bool shouldLeaveBox() override;
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

protected:
    int m_index;

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

    bool m_available = false;
    size_t m_availabilityTimerKey;
};
