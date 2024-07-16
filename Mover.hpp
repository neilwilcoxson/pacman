#pragma once

#include <SDL.h>

#include "util.hpp"

// forward declaration
typedef BoardLayout;

class Mover
{
public:
    Mover(int startRow, int startCol, Direction startFacing);
    virtual void draw(SDL_Renderer* renderer, BoardLayout& board) = 0;
    virtual void handleWall() = 0;
    virtual void handleArrival(BoardLayout& board) {};
    void changeDirection(BoardLayout& board, Direction newDirection);
    void handleMovement(BoardLayout& board);
    void relocate(int row, int col);
protected:
    int m_row;
    int m_col;
    int m_xPixelOffset = 0;
    int m_yPixelOffset = 0;
    Direction m_facingDirection = Direction::LEFT;
    Direction m_pendingDirection = Direction::LEFT;
    uint64_t m_lastDrawnTicks = 0;
    int m_velocity = 100;
    std::string m_name;
};

class Pacman : public Mover
{
public:
    Pacman(int startRow, int startCol, Direction startFacing);
    void draw(SDL_Renderer* renderer, BoardLayout& board) override;
    void handleWall() override;
    void handleArrival(BoardLayout& board) override;
private:
    static inline const SDL_Color COLOR { 0xff, 0xff, 0, SDL_ALPHA_OPAQUE };
    static inline const int RADIUS = 10;

    int m_normalDotPoints = 10;
    int m_superDotPoints = 100;

    uint64_t m_score = 0;
};

class Ghost : public Mover
{
public:
    Ghost(int startRow, int startCol, Direction startFacing, const SDL_Color& color, const std::string& name);
    void draw(SDL_Renderer* renderer, BoardLayout& board) override;
    void handleWall() override;
public:
    bool inBox = true;
private:
    SDL_Color m_color;
    bool m_isFlashing = false;
};
