#pragma once

#include <SDL.h>

#include "util.hpp"

// forward declaration
class GameState;

class Mover
{
public:
    Mover(GameState& gameState, int startRow, int startCol, Direction startFacing);
    virtual void update() = 0;
    virtual void handleWall() = 0;
    virtual void handleArrival() {};
    virtual void reset() = 0;
    void changeDirection(Direction newDirection);
    void handleMovement();
    bool hasSamePositionAs(const Mover& otherMover) const;
    void relocate(int row, int col);
protected:
    int m_row;
    int m_col;
    int m_xPixelOffset = 0; // offset from center within the column
    int m_yPixelOffset = 0; // offset from center within the row
    Direction m_facingDirection = Direction::LEFT;
    Direction m_pendingDirection = Direction::LEFT;
    uint64_t m_lastDrawnTicks = 0;
    int m_velocity = 100; // pixels per second (might need to change based on resizable window)
    std::string m_name;
    GameState& m_gameState;
};

class Pacman : public Mover
{
public:
    Pacman(GameState& gameState);
    void update() override;
    void handleWall() override;
    void handleArrival() override;
    void reset() override;
private:
    static inline const int PACMAN_START_ROW = 23;
    static inline const int PACMAN_START_COL = 14;
    static inline const Direction PACMAN_START_DIRECTION = Direction::LEFT;
    static inline const SDL_Color COLOR { 0xff, 0xff, 0, SDL_ALPHA_OPAQUE };
    static inline const int RADIUS = 10;

    friend class GameState;
};

class Ghost : public Mover
{
public:
    static std::vector<Ghost> makeGhosts(GameState& gameState);
    static void resetGhosts(std::vector<Ghost>& ghosts);
    Ghost(GameState& gameState, int startRow, int startCol, Direction startFacing, const SDL_Color& color, const std::string& name);
    void update() override;
    void handleWall() override;
    void reset() override;
public:
    bool inBox = true;
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

    uint64_t m_flashingDeadlineTicks = 0;
    int m_flashColorIndex = 0;

    friend class Pacman;
};
