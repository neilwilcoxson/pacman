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

void Pacman::draw()
{
    handleMovement();
    drawFilledCircle(m_gameState.m_renderer, X_CENTER(m_col) + m_xPixelOffset, Y_CENTER(m_row) + m_yPixelOffset, RADIUS, COLOR);
}

void Pacman::handleArrival()
{
    // if a key is being held down, attempt to handle it
    // the main loop handles the direction change if pacman is stopped
    int numKeys = 0;
    const uint8_t* const keys = SDL_GetKeyboardState(&numKeys);
    if (numKeys > 0)
    {
        if(keys[SDL_SCANCODE_UP]) changeDirection(Direction::UP);
        else if(keys[SDL_SCANCODE_DOWN]) changeDirection(Direction::DOWN);
        else if(keys[SDL_SCANCODE_LEFT]) changeDirection(Direction::LEFT);
        else if(keys[SDL_SCANCODE_RIGHT]) changeDirection(Direction::RIGHT);
    }

    for(auto& ghost : m_gameState.m_ghosts)
    {
        if (ghost.m_row == m_row && ghost.m_col == m_col)
        {
            if(ghost.m_isFlashing)
            {
                m_score += m_gameState.m_flashingGhostPoints;
                m_gameState.m_flashingGhostPoints *= 2;
                ghost.relocate(Ghost::GHOST_START_ROW, Ghost::GHOST_START_COL);
                ghost.inBox = true;
            }
            else
            {
                LOG_INFO("Found a ghost, lose a life: %d -> %d", m_lives, m_lives - 1);
                m_lives--;

                m_row = PACMAN_START_ROW;
                m_col = PACMAN_START_COL;
                m_facingDirection = PACMAN_START_DIRECTION;
                Ghost::resetGhosts(m_gameState.m_ghosts);
            }
        }
    }

    switch (m_gameState.m_board[m_row][m_col])
    {
    case DOT:
        m_score += m_normalDotPoints;
        m_gameState.m_board[m_row][m_col] = ' ';
        break;
    case SUPER_DOT:
        m_score += m_superDotPoints;
        m_gameState.m_board[m_row][m_col] = ' ';
        for(auto& ghost : m_gameState.m_ghosts)
        {
            ghost.m_isFlashing = true;
            m_gameState.m_flashingGhostDeadline = SDL_GetTicks64() + m_gameState.m_flashingGhostDurationMs;
        }
        break;
    case WRAP:
        m_col = m_gameState.m_board[0].size() - m_col - 1;
        break;
    default:
        return;
    }

    LOG_INFO("Score: %llu", m_score);
}

void Pacman::handleWall()
{
    LOG_TRACE("Pacman: hit wall at row=%d col=%d", m_row, m_col);
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

void Ghost::resetGhosts(std::vector<Ghost>& ghosts)
{
    int index = 0;
    for(auto& ghost : ghosts)
    {
        ghost.relocate(GHOST_START_ROW, GHOST_START_COL + index);
        ghost.inBox = true;
        index++;
    }
}

Ghost::Ghost(GameState& gameState, int startRow, int startCol, Direction startFacing, const SDL_Color& color, const std::string& name)
: Mover(gameState, startRow, startCol, startFacing), m_color(color)
{
    m_name = name;
    m_velocity = 50;
}

void Ghost::draw()
{
    SDL_Rect rect;
    rect.h = 20;
    rect.w = 20;
    rect.x = X_CENTER(m_col) + m_xPixelOffset;
    rect.y = Y_CENTER(m_row) + m_yPixelOffset;

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

    SDL_SetRenderDrawColor(m_gameState.m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_gameState.m_renderer, &rect);
}

void Ghost::handleWall()
{
    LOG_TRACE("%s hits wall", m_name.c_str());
    // TODO ghosts always get stuck in the corner, add more logic here
    m_pendingDirection = (Direction)(((size_t)m_facingDirection + 1) % (size_t)Direction::MAX);
}
