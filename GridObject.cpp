#include "GameState.hpp"
#include "GridObject.hpp"
#include "util.hpp"

GridObject::GridObject(GameState& gameState, int row, int col)
: m_row(row), m_col(col), m_gameState(gameState)
{

}

bool GridObject::hasSamePositionAs(const GridObject& otherObject) const
{
    return m_row == otherObject.m_row && m_col == otherObject.m_col;
}

void GridObject::relocate(int row, int col)
{
    m_row = row;
    m_col = col;
}

bool directionValid();

Mover::Mover(GameState& gameState, int startRow, int startCol, Direction startFacing)
: GridObject(gameState, startRow, startCol), m_facingDirection(startFacing)
{
    m_lastDrawnTicks = SDL_GetTicks64();
}

void Mover::changeDirection(Direction newDirection)
{
    
    if (directionValid(newDirection))
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

bool Mover::directionValid(Direction newDirection)
{
    int newRow = m_row + Y_INCREMENT[(size_t)newDirection];
    int newCol = m_col + X_INCREMENT[(size_t)newDirection];
    return m_gameState.m_board[newRow][newCol] != BOUNDARY;
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
    m_awayFromPacmanDirectionInterval += m_index;
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

    // TODO store ghost in full res instead of scaling
    const std::vector<std::string> GHOST_GRID =
    {
        "    xx    ",
        "  xxxxxx  ",
        " xx xx xx ",
        " xxxxxxxx ",
        " xxxxxxxx ",
        " xxxxxxxx ",
        " xxxxxxxx ",
        " x  x x x "   
    };

    const int SCALING_FACTOR = 3;
    std::vector<std::string> scaledGhost;

    for (const auto& line : GHOST_GRID)
    {
        std::string newString;
        for (const auto& c : line)
        {
            for (int i = 0; i < SCALING_FACTOR; i++)
            {
                newString += c;
            }
        }

        for (int i = 0; i < SCALING_FACTOR; i++)
        {
            scaledGhost.push_back(newString);
        }
    }

    SDL_SetRenderDrawColor(m_gameState.m_renderer, color.r, color.g, color.b, color.a);

    for(int row = 0; row < scaledGhost.size(); row++)
    {
        for(int col = 0; col < scaledGhost[0].size(); col++)
        {
            if(scaledGhost[row][col] == 'x')
            {
                SDL_RenderDrawPoint(
                    m_gameState.m_renderer,
                    X_CENTER(m_col) + m_xPixelOffset + col - scaledGhost.size() / 2,
                    Y_CENTER(m_row) + m_yPixelOffset + row - scaledGhost.size() / 2);
            }
        }
    }
}

void Ghost::handleWall()
{
    LOG_TRACE("%s hits wall", m_name.c_str());
    m_pendingDirection = (Direction)(((size_t)m_facingDirection + 1) % (size_t)Direction::MAX);
}

void Ghost::handleArrival()
{
    for (size_t newDirection = 0; newDirection < (size_t)Direction::MAX; newDirection++)
    {
        if (directionValid((Direction)newDirection))
        {
            if (abs(m_row + Y_INCREMENT[newDirection] - m_gameState.m_pacman.m_row) < abs(m_row - m_gameState.m_pacman.m_row)
                || abs(m_col + X_INCREMENT[newDirection] - m_gameState.m_pacman.m_col) < abs(m_col - m_gameState.m_pacman.m_col))
            {
                // this direction gets the ghost closer to pacman
                if(m_numMovesTowardPacman < m_awayFromPacmanDirectionInterval)
                {
                    m_numMovesTowardPacman++;
                    m_pendingDirection = m_facingDirection = (Direction)newDirection;
                    return;
                }
            }
            else if(m_numMovesTowardPacman >= m_awayFromPacmanDirectionInterval)
            {
                m_numMovesTowardPacman = 0;
                m_pendingDirection = m_facingDirection = (Direction)newDirection;
                return;
            }
        }
    }
}

void Ghost::reset()
{
    relocate(GHOST_START_ROW, GHOST_START_COL + m_index);
    m_inBox = true;
    m_isFlashing = false;
}

std::vector<DisplayFruit> DisplayFruit::makeDisplayFruits(GameState& gameState)
{
    int index = 0;
    std::vector<DisplayFruit> displayFruits;
    displayFruits.reserve(4);
    displayFruits.emplace_back(gameState, index++);
    displayFruits.emplace_back(gameState, index++);
    displayFruits.emplace_back(gameState, index++);
    return displayFruits;
}

DisplayFruit::DisplayFruit(GameState& gameState, int index)
: GridObject(gameState, FRUIT_DISPLAY_ROW, FRUIT_DISPLAY_START_COL - index)
{
    m_name = std::string("fruit ") + std::to_string(index);
}

void DisplayFruit::update()
{
    SDL_Rect rect;
    rect.h = 20;
    rect.w = 20;
    rect.x = X_CENTER(m_col) + m_xPixelOffset - rect.w / 2;
    rect.y = Y_CENTER(m_row) + m_yPixelOffset - rect.h / 2;
    SDL_Color color = COLOR_RED;
    SDL_SetRenderDrawColor(m_gameState.m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_gameState.m_renderer, &rect);
}

PointsFruit::PointsFruit(GameState& gameState)
: DisplayFruit(gameState, -1)
{
    m_row = FRUIT_SPAWN_ROW;
    m_col = FRUIT_SPAWN_COL;
}

void PointsFruit::update()
{
    if(!m_available)
    {
        return;
    }

    if(SDL_GetTicks64() > m_fruitDeadlineTicks)
    {
        m_available = false;
        return;
    }

    DisplayFruit::update();
}

void PointsFruit::reset()
{
    m_available = false;
}

void PointsFruit::activate()
{
    m_available = true;
    m_fruitDeadlineTicks = SDL_GetTicks64() + FRUIT_DURATION_TICKS;
}
