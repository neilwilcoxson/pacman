#include "GameState.hpp"
#include "Mover.hpp"
#include "util.hpp"

static const int X_INCREMENT[] = { 0, 0, -1, 1, 0 };
static const int Y_INCREMENT[] = { -1, 1, 0, 0, 0 };

Mover::Mover(int startRow, int startCol, Direction startFacing)
: m_row(startRow), m_col(startCol), m_facingDirection(startFacing)
{
    m_lastDrawnTicks = SDL_GetTicks64();
}

void Mover::changeDirection(BoardLayout& board, Direction newDirection)
{
    int newRow = m_row + Y_INCREMENT[(size_t)newDirection];
    int newCol = m_col + X_INCREMENT[(size_t)newDirection];
    if (board[newRow][newCol] != BOUNDARY)
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

void Mover::handleMovement(BoardLayout& board)
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

    if (board[nextRow][nextCol] == BOUNDARY)
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

    if (board[nextRow][nextCol] == BOUNDARY)
    {
        m_xPixelOffset = 0;
        m_yPixelOffset = 0;
        handleWall();
        return;
    }

    m_row = nextRow;
    m_col = nextCol;
    handleArrival(board);

    if (xIncrement == -1) { m_xPixelOffset = maxXOffset - (m_xPixelOffset - minXOffset); m_yPixelOffset = 0; }
    if (xIncrement == 1) { m_xPixelOffset = minXOffset + (m_xPixelOffset - maxXOffset);  m_yPixelOffset = 0; }
    if (yIncrement == -1) { m_yPixelOffset = maxYOffset - (m_yPixelOffset - minYOffset); m_xPixelOffset = 0; }
    if (yIncrement == 1) { m_yPixelOffset = minYOffset + (m_yPixelOffset - maxXOffset); m_xPixelOffset = 0; }
}

void Mover::relocate(int row, int col)
{
    m_row = row;
    m_col = col;
}

Pacman::Pacman(int startRow, int startCol, Direction startFacing)
: Mover(startRow, startCol, startFacing)
{
    m_velocity = 200;
    m_name = "pacman";
}

void Pacman::draw(SDL_Renderer* renderer, BoardLayout& board)
{
    handleMovement(board);
    drawFilledCircle(renderer, X_CENTER(m_col) + m_xPixelOffset, Y_CENTER(m_row) + m_yPixelOffset, RADIUS, COLOR);
}

void Pacman::handleArrival(BoardLayout& board)
{
    // if a key is being held down, attempt to handle it
    // the main loop handles the direction change if pacman is stopped
    int numKeys = 0;
    const uint8_t* const keys = SDL_GetKeyboardState(&numKeys);
    if (numKeys > 0)
    {
        if(keys[SDL_SCANCODE_UP]) changeDirection(board, Direction::UP);
        else if(keys[SDL_SCANCODE_DOWN]) changeDirection(board, Direction::DOWN);
        else if(keys[SDL_SCANCODE_LEFT]) changeDirection(board, Direction::LEFT);
        else if(keys[SDL_SCANCODE_RIGHT]) changeDirection(board, Direction::RIGHT);
    }

    switch (board[m_row][m_col])
    {
    case DOT:
        m_score += m_normalDotPoints;
        board[m_row][m_col] = ' ';
        break;
    case SUPER_DOT:
        m_score += m_superDotPoints;
        board[m_row][m_col] = ' ';
        break;
    case WRAP:
        m_col = board[0].size() - m_col - 1;
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

Ghost::Ghost(int startRow, int startCol, Direction startFacing, const SDL_Color& color, const std::string& name)
: Mover(startRow, startCol, startFacing), m_color(color)
{
    m_name = name;
    m_velocity = 50;
}

void Ghost::draw(SDL_Renderer* renderer, BoardLayout& board)
{
    SDL_Rect rect;
    rect.h = 20;
    rect.w = 20;
    rect.x = X_CENTER(m_col) + m_xPixelOffset;
    rect.y = Y_CENTER(m_row) + m_yPixelOffset;

    handleMovement(board);

    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void Ghost::handleWall()
{
    LOG_TRACE("%s hits wall", m_name.c_str());
    // TODO ghosts always get stuck in the corner, add more logic here
    m_pendingDirection = (Direction)(((size_t)m_facingDirection + 1) % (size_t)Direction::MAX);
}
