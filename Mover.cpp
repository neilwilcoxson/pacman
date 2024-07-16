#include "GameState.hpp"
#include "Mover.hpp"
#include "util.hpp"

static const int X_INCREMENT[] = { 0, 0, -1, 1, 0 };
static const int Y_INCREMENT[] = { -1, 1, 0, 0, 0 };

Mover::Mover(GameState& gameState, int startRow, int startCol, Direction startFacing)
: m_row(startRow), m_col(startCol), m_facingDirection(startFacing), m_gameState(gameState)
{
    m_lastDrawnTicks = SDL_GetTicks64();
}

void Mover::changeDirection(Direction newDirection)
{
    int newRow = m_row + Y_INCREMENT[(size_t)newDirection];
    int newCol = m_col + X_INCREMENT[(size_t)newDirection];
    if (m_gameState.m_board[newRow][newCol] != BOUNDARY)
    {
        LOG_DEBUG("Changing pending direction (%s) -> (%s)",
            DIRECTION_AS_STRING[(size_t)m_pendingDirection],
            DIRECTION_AS_STRING[(size_t)newDirection]);
        m_pendingDirection = newDirection;
    }
    else
    {
        LOG_DEBUG("Rejected pending direction change (%s) -> (%s)",
            DIRECTION_AS_STRING[(size_t)m_pendingDirection],
            DIRECTION_AS_STRING[(size_t)newDirection]);
    }
}

void Mover::handleMovement()
{
    uint64_t currentTicks = SDL_GetTicks64();
    int numPixelsToMove = (currentTicks - m_lastDrawnTicks) * m_velocity / 1000;
    if (numPixelsToMove == 0)
    {
        // no changes would take place if there is no velocity
        return;
    }
    m_lastDrawnTicks = currentTicks;

    int xIncrement = X_INCREMENT[(size_t)m_facingDirection];
    int yIncrement = Y_INCREMENT[(size_t)m_facingDirection];

    int nextRow = m_row + yIncrement;
    int nextCol = m_col + xIncrement;

    int minXOffset = -TILE_WIDTH / 2;
    int maxXOffset = TILE_WIDTH / 2;
    int minYOffset = -TILE_HEIGHT / 2;
    int maxYOffset = TILE_HEIGHT / 2;

    if (m_gameState.m_board[nextRow][nextCol] == BOUNDARY)
    {
        // restrict movement to the center of the last tile before a boundary
        if (xIncrement == -1) minXOffset = 0;
        if (xIncrement == 1) maxXOffset = 0;
        if (yIncrement == -1) minYOffset = 0;
        if (yIncrement == 1) maxYOffset = 0;

        LOG_TRACE("%s: about to hit wall going %s, new ranges xOffset: [%d, %d] yOffset: [%d, %d]", m_name.c_str(),
            DIRECTION_AS_STRING[(size_t)m_facingDirection], minXOffset, maxXOffset, minYOffset, maxYOffset);
    }

    if (xIncrement != 0)
    {
        m_xPixelOffset += xIncrement * numPixelsToMove;
        if (m_xPixelOffset >= minXOffset && m_xPixelOffset <= maxXOffset)
        {
            // return early if we stay on the same grid space
            return;
        }
    }

    if (yIncrement != 0)
    {
        m_yPixelOffset += yIncrement * numPixelsToMove;
        if (m_yPixelOffset >= minYOffset && m_yPixelOffset <= maxYOffset)
        {
            // return early if we stay on the same grid space
            return;
        }
    }

    if (m_facingDirection != m_pendingDirection)
    {
        m_facingDirection = m_pendingDirection;
        m_xPixelOffset = 0;
        m_yPixelOffset = 0;
        return;
    }

    if (m_gameState.m_board[nextRow][nextCol] == BOUNDARY)
    {
        m_xPixelOffset = 0;
        m_yPixelOffset = 0;
        handleWall();
        return;
    }

    m_row = nextRow;
    m_col = nextCol;
    handleArrival();

    if (xIncrement == -1) { m_xPixelOffset = maxXOffset - (m_xPixelOffset - minXOffset); m_yPixelOffset = 0; }
    if (xIncrement == 1) { m_xPixelOffset = minXOffset + (m_xPixelOffset - maxXOffset);  m_yPixelOffset = 0; }
    if (yIncrement == -1) { m_yPixelOffset = maxYOffset - (m_yPixelOffset - minYOffset); m_xPixelOffset = 0; }
    if (yIncrement == 1) { m_yPixelOffset = minYOffset + (m_yPixelOffset - maxYOffset); m_xPixelOffset = 0; }
}

bool Mover::hasSamePositionAs(const Mover& otherMover) const
{
    return m_row == otherMover.m_row && m_col == otherMover.m_col;
}

void Mover::relocate(int row, int col)
{
    m_row = row;
    m_col = col;
}

Pacman::Pacman(GameState& gameState)
: Mover(gameState, PACMAN_START_ROW, PACMAN_START_COL, PACMAN_START_DIRECTION)
{
    m_velocity = 200;
    m_name = "pacman";
}

void Pacman::update()
{
    handleMovement();
    drawFilledCircle(m_gameState.m_renderer, X_CENTER(m_col) + m_xPixelOffset, Y_CENTER(m_row) + m_yPixelOffset, RADIUS, COLOR);
}

void Pacman::handleArrival()
{
    m_gameState.handlePacmanArrival();
}

void Pacman::handleWall()
{
    LOG_TRACE("Pacman: hit wall at row=%d col=%d", m_row, m_col);
}

void Pacman::reset()
{
    m_row = PACMAN_START_ROW;
    m_col = PACMAN_START_COL;
    m_facingDirection = PACMAN_START_DIRECTION;
}

std::vector<Ghost> Ghost::makeGhosts(GameState& gameState)
{
    std::vector<Ghost> ghosts;
    ghosts.reserve(NUM_GHOSTS);
    ghosts.emplace_back(gameState, GHOST_START_ROW, GHOST_START_COL + 0, GHOST_START_DIRECTION, COLOR_RED, "Blinky");
    ghosts.emplace_back(gameState, GHOST_START_ROW, GHOST_START_COL + 1, GHOST_START_DIRECTION, COLOR_PINK, "Pinky");
    ghosts.emplace_back(gameState, GHOST_START_ROW, GHOST_START_COL + 2, GHOST_START_DIRECTION, COLOR_TURQUOISE, "Inky");
    ghosts.emplace_back(gameState, GHOST_START_ROW, GHOST_START_COL + 3, GHOST_START_DIRECTION, COLOR_ORANGE, "Clyde");
    return ghosts;
}

Ghost::Ghost(GameState& gameState, int startRow, int startCol, Direction startFacing, const SDL_Color& color, const std::string& name)
: Mover(gameState, startRow, startCol, startFacing), m_color(color), m_index(nextIndex++)
{
    m_name = name;
    m_velocity = 50;
}

void Ghost::update()
{
    handleMovement();

    SDL_Color color = m_color;
    if(m_isFlashing)
    {
        uint64_t currentTicks = SDL_GetTicks64();
        if(currentTicks > m_flashingDeadlineTicks)
        {
            m_flashingDeadlineTicks = currentTicks + 1000;
            m_flashColorIndex = 1 - m_flashColorIndex;
        }

        color = FLASH_COLOR[m_flashColorIndex];
    }

    SDL_Rect rect;
    rect.h = 20;
    rect.w = 20;
    rect.x = X_CENTER(m_col) + m_xPixelOffset;
    rect.y = Y_CENTER(m_row) + m_yPixelOffset;

    SDL_SetRenderDrawColor(m_gameState.m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_gameState.m_renderer, &rect);
}

void Ghost::handleWall()
{
    LOG_TRACE("%s hits wall", m_name.c_str());
    // TODO ghosts always get stuck in the corner, add more logic here
    m_pendingDirection = (Direction)(((size_t)m_facingDirection + 1) % (size_t)Direction::MAX);
}

void Ghost::reset()
{
    relocate(GHOST_START_ROW, GHOST_START_COL + m_index);
    inBox = true;
    m_isFlashing = false;
}
