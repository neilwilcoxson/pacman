#include "GameState.hpp"
#include "GridObject.hpp"
#include "TimerService.hpp"
#include "util.hpp"

GridObject::GridObject(GameState& gameState, int row, int col) : m_row(row), m_col(col), m_gameState(gameState)
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

Mover::Mover(GameState& gameState, int startRow, int startCol, Direction startFacing)
: GridObject(gameState, startRow, startCol), m_facingDirection(startFacing)
{
    m_lastDrawnTicks = SDL_GetTicks64();
}

void Mover::changeDirection(Direction newDirection)
{
    if(directionValid(newDirection))
    {
        LOG_DEBUG(
            "Changing pending direction (%s) -> (%s)",
            DIRECTION_AS_STRING[(size_t)m_pendingDirection],
            DIRECTION_AS_STRING[(size_t)newDirection]);
        m_pendingDirection = newDirection;
    }
    else
    {
        LOG_DEBUG(
            "Rejected pending direction change (%s) -> (%s)",
            DIRECTION_AS_STRING[(size_t)m_pendingDirection],
            DIRECTION_AS_STRING[(size_t)newDirection]);
    }
}

void Mover::handleMovement()
{
    uint64_t currentTicks = SDL_GetTicks64();
    int numPixelsToMove = (int)(currentTicks - m_lastDrawnTicks) * m_velocity / 1000;
    if(numPixelsToMove == 0)
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

    if(m_gameState.m_board[nextRow][nextCol] == BOUNDARY)
    {
        // restrict movement to the center of the last tile before a boundary
        if(xIncrement == -1)
            minXOffset = 0;
        if(xIncrement == 1)
            maxXOffset = 0;
        if(yIncrement == -1)
            minYOffset = 0;
        if(yIncrement == 1)
            maxYOffset = 0;

        LOG_TRACE(
            "%s: about to hit wall going %s, new ranges xOffset: [%d, %d] yOffset: [%d, %d]",
            m_name.c_str(),
            DIRECTION_AS_STRING[(size_t)m_facingDirection],
            minXOffset,
            maxXOffset,
            minYOffset,
            maxYOffset);
    }

    if(xIncrement != 0)
    {
        m_xPixelOffset += xIncrement * numPixelsToMove;
        if(m_xPixelOffset >= minXOffset && m_xPixelOffset <= maxXOffset)
        {
            // return early if we stay on the same grid space
            return;
        }
    }

    if(yIncrement != 0)
    {
        m_yPixelOffset += yIncrement * numPixelsToMove;
        if(m_yPixelOffset >= minYOffset && m_yPixelOffset <= maxYOffset)
        {
            // return early if we stay on the same grid space
            return;
        }
    }

    if(m_facingDirection != m_pendingDirection)
    {
        m_facingDirection = m_pendingDirection;
        m_xPixelOffset = 0;
        m_yPixelOffset = 0;
        return;
    }

    if(m_gameState.m_board[nextRow][nextCol] == BOUNDARY)
    {
        m_xPixelOffset = 0;
        m_yPixelOffset = 0;
        handleWall();
        return;
    }

    m_row = nextRow;
    m_col = nextCol;

    // adjust pixel offset to be relative to the new row and column
    m_xPixelOffset -= xIncrement * (maxXOffset - minXOffset);
    m_yPixelOffset -= yIncrement * (maxYOffset - minYOffset);

    handleArrival();
}

bool Mover::directionValid(const Direction newDirection) const
{
    int newRow = m_row + Y_INCREMENT[(size_t)newDirection];
    int newCol = m_col + X_INCREMENT[(size_t)newDirection];
    return m_gameState.m_board[newRow][newCol] != BOUNDARY;
}

bool Mover::directionIsCloser(const Direction newDirection, const GridPosition& otherPosition) const
{
    const size_t newDirIndex = (size_t)newDirection;
    return abs(m_row + Y_INCREMENT[newDirIndex] - otherPosition.row) < abs(m_row - otherPosition.row)
           || abs(m_col + X_INCREMENT[newDirIndex] - otherPosition.col) < abs(m_col - otherPosition.col);
}

void Pacman::drawPacman(
    SDL_Renderer* renderer,
    const int xCenter,
    const int yCenter,
    const Direction facingDirection,
    const int mouthPixels)
{
    SDL_SetRenderDrawColor(renderer, COLOR.r, COLOR.g, COLOR.b, COLOR.a);

    int xIncrement = X_INCREMENT[(size_t)facingDirection];
    int yIncrement = Y_INCREMENT[(size_t)facingDirection];
    for(int x = 0; x < RADIUS * 2; x++)
    {
        for(int y = 0; y < RADIUS * 2; y++)
        {
            int dx = RADIUS - x;
            int dy = RADIUS - y;
            if(dx * dx + dy * dy <= RADIUS * RADIUS)
            {
                if(dy * yIncrement >= 0 && dx * xIncrement < mouthPixels && abs(dy) > abs(dx) && yIncrement != 0
                   || dx * xIncrement >= 0 && dy * yIncrement < mouthPixels && abs(dx) > abs(dy) && xIncrement != 0)
                {
                    continue;
                }
                SDL_RenderDrawPoint(renderer, xCenter + dx, yCenter + dy);
            }
        }
    }
}

Pacman::Pacman(GameState& gameState) : Mover(gameState, PACMAN_START_ROW, PACMAN_START_COL, PACMAN_START_DIRECTION)
{
    m_velocity = 300;
    m_name = "pacman";
}

void Pacman::update()
{
    handleMovement();

    int xCenter = X_CENTER(m_col) + m_xPixelOffset;
    int yCenter = Y_CENTER(m_row) + m_yPixelOffset;
    drawPacman(m_gameState.m_renderer, xCenter, yCenter, m_facingDirection, m_mouthPixels);

    if(abs(m_mouthPixels) >= RADIUS)
    {
        m_mouthIncrement *= -1;
    }
    m_mouthPixels += m_mouthIncrement;
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
    m_lastDrawnTicks = SDL_GetTicks64();
}

std::vector<std::unique_ptr<Ghost>> Ghost::makeGhosts(GameState& gameState)
{
    std::vector<std::unique_ptr<Ghost>> ghosts;
    ghosts.reserve(NUM_GHOSTS);
    ghosts.emplace_back(std::make_unique<Blinky>(
        gameState, GHOST_START_ROW, GHOST_START_COL + 0, GHOST_START_DIRECTION, COLOR_RED, "Blinky"));
    ghosts.emplace_back(std::make_unique<Pinky>(
        gameState, GHOST_START_ROW, GHOST_START_COL + 1, GHOST_START_DIRECTION, COLOR_PINK, "Pinky"));
    ghosts.emplace_back(std::make_unique<Inky>(
        gameState, GHOST_START_ROW, GHOST_START_COL + 2, GHOST_START_DIRECTION, COLOR_TURQUOISE, "Inky"));
    ghosts.emplace_back(std::make_unique<Clyde>(
        gameState, GHOST_START_ROW, GHOST_START_COL + 3, GHOST_START_DIRECTION, COLOR_ORANGE, "Clyde"));
    return ghosts;
}

Ghost::Ghost(
    GameState& gameState,
    int startRow,
    int startCol,
    Direction startFacing,
    const SDL_Color& color,
    const std::string& name)
: Mover(gameState, startRow, startCol, startFacing), m_color(color), m_index(nextIndex++)
{
    m_name = name;
    m_velocity = 100;
}

void Ghost::update()
{
    handleMovement();

    SDL_Color color = m_color;
    if(m_isFlashing)
    {
        color = FLASH_COLOR[m_flashColorIndex];
    }

    // clang-format off
    const std::vector<std::string> GHOST_GRID =
    {
        "     xxxx     ",
        "   xxxxxxxx   ",
        "  xxxxxxxxxx  ",
        " xxxxxxxxxxxx ",
        " xxxxxxxxxxxx ",
        " xxxxxxxxxxxx ",
        " xxxxxxxxxxxx ",
        "xxxxxxxxxxxxxx",
        "xxxxxxxxxxxxxx",
        "xxxxxxxxxxxxxx",
        "xxxxxxxxxxxxxx",
        "xxxxxxxxxxxxxx",
        "xxxxxxxxxxxxxx",
        "xx xxx  xxx xx",
        "x   xx  xx   x"
    };
    // clang-format on

    const int SCALING_FACTOR = 2;
    std::vector<std::string> scaledGhost;

    for(const auto& line : GHOST_GRID)
    {
        std::string newString;
        for(const auto& c : line)
        {
            for(int i = 0; i < SCALING_FACTOR; i++)
            {
                newString += c;
            }
        }

        for(int i = 0; i < SCALING_FACTOR; i++)
        {
            scaledGhost.push_back(newString);
        }
    }

    SDL_SetRenderDrawColor(m_gameState.m_renderer, color.r, color.g, color.b, color.a);

    for(int row = 0; row < scaledGhost.size(); row++)
    {
        for(int col = 0; col < scaledGhost[row].length(); col++)
        {
            if(scaledGhost[row][col] == 'x')
            {
                SDL_RenderDrawPoint(
                    m_gameState.m_renderer,
                    X_CENTER(m_col) + m_xPixelOffset + col - (int)scaledGhost[row].length() / 2,
                    Y_CENTER(m_row) + m_yPixelOffset + row - (int)scaledGhost.size() / 2);
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
    switch(m_mode)
    {
    case Mode::CHASE:
        calculateTargetLocation();
        break;
    case Mode::FRIGHTENED:
        m_targetLocation = m_defaultTargetLocation;
        break;
    case Mode::SCATTER:
        break;
    default:
        break;
    }

    for(size_t newDirIndex = 0; newDirIndex < (size_t)Direction::MAX; newDirIndex++)
    {
        Direction newDirection = (Direction)newDirIndex;
        if(directionValid(newDirection))
        {
            if(m_mode == Mode::SCATTER || directionIsCloser(newDirection, m_targetLocation))
            {
                m_pendingDirection = newDirection;
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

void Ghost::handleSuperDot()
{
    m_isFlashing = true;

    auto& timerService = TimerService::getInstance();

    // TODO this timer never stops?
    size_t flashColorTimerKey =
        timerService.addTimer(1000, true, [this]() { m_flashColorIndex = 1 - m_flashColorIndex; });
    timerService.startTimer(flashColorTimerKey);

    size_t flashingGhostTimerKey = timerService.addTimer(
        m_gameState.m_flashingGhostDurationMs,
        false,
        [this]()
        {
            m_gameState.m_flashingGhostPoints = m_gameState.DEFAULT_FLASHING_GHOST_POINTS;
            m_isFlashing = false;
        });
    timerService.startTimer(flashingGhostTimerKey);
}

void Blinky::calculateTargetLocation()
{
    // TODO mode change
    //
    // Scatter for 7 seconds, then Chase for 20 seconds.
    // Scatter for 7 seconds, then Chase for 20 seconds.
    // Scatter for 5 seconds, then Chase for 20 seconds.
    // Scatter for 5 seconds, then switch to Chase mode permanently.

    // TODO when to leave the box logic: start outside
    // TODO shouldn't need to set this every time
    m_defaultTargetLocation = {0, 0};
    m_targetLocation = m_gameState.m_pacman.getPosition();
}

void Pinky::calculateTargetLocation()
{
    // TODO when to leave the box logic: leave immediately
    m_defaultTargetLocation = {0, 30};
    // try to move two tiles ahead of pacman
    auto pacmanLocation = m_gameState.m_pacman.getPosition();
    auto pacmanDirection = m_gameState.m_pacman.getDirection();
    m_targetLocation.row = pacmanLocation.row + 4 * Y_INCREMENT[(size_t)pacmanDirection];
    m_targetLocation.col = pacmanLocation.col + 4 * X_INCREMENT[(size_t)pacmanDirection];
}

void Inky::calculateTargetLocation()
{
    // TODO when to leave the box logic: 30 dots
    m_defaultTargetLocation = {32, 0};
    auto pacmanLocation = m_gameState.m_pacman.getPosition();
    auto redLocation = m_gameState.m_ghosts[0]->getPosition();
    m_targetLocation.row = 3 * redLocation.row - 2 * pacmanLocation.row;
    m_targetLocation.col = 3 * redLocation.col - 2 * pacmanLocation.col;
}

void Clyde::calculateTargetLocation()
{
    // TODO when to leave the box logic: 1/3 dots eaten
    m_defaultTargetLocation = {32, 32};
    auto pacmanLocation = m_gameState.m_pacman.getPosition();
    int distance = abs(pacmanLocation.row - m_row) + abs(pacmanLocation.col - m_col);
    if(distance > 8)
    {
        m_targetLocation = pacmanLocation;
    }
    else
    {
        m_targetLocation = m_defaultTargetLocation;
    }
}

static const std::string FRUIT_SPRITES =
    // cherry
    "            "
    "          BB"
    "        BBBB"
    "      BB B  "
    "     B   B  "
    " RRRB   B   "
    "RRRBRR B    "
    "RRRRR RBR   "
    "RWRR RRBRRR "
    "RRWR RRRRRR "
    " RRR RWRRRR "
    "     RRWRRR "
    "      RRRR  "
    // strawberry
    "            "
    "      W     "
    "   GGGWGGG  "
    "  RRGGGGGRR "
    " RRRRRGRRRRR"
    " RWRRRRRWRRR"
    " RRRWRWRRRRR"
    " RRRRRRRRWRR"
    "  RWRRWRRRRR"
    "  RRRRRRRRR "
    "   RRWRRW   "
    "    RRRRR   "
    "      R     "
    // orange
    "            "
    "       GG   "
    "     BGGGGG "
    "     B GGG  "
    "  TTBBBTTT  "
    " TTTTBTTTTT "
    "TTTTTTTTTTTT"
    "TTTTTTTTTTTT"
    "TTTTTTTTTTTT"
    "TTTTTTTTTTTT"
    " TTTTTTTTTT "
    " TTTTTTTTTT "
    "  TTTTTTTT  "
    // apple
    "            "
    "      B     "
    " RRR B RRR  "
    "RRRRRBRRRRR "
    "RRRRRRRRRRRR"
    "RRRRRRRRRRRR"
    "RRRRRRRRRRRR"
    "RRRRRRRRRWRR"
    "RRRRRRRRRWRR"
    " RRRRRRRWRR "
    " RRRRRRRRRR "
    "  RRRRRRRR  "
    "   RR RRR   "
    // melon
    "   t        "
    "    ttttt   "
    "      t     "
    "     GWG    "
    "   GGGtGGG  "
    "  GtWtGGGtG "
    "  GGtGGWtGG "
    " GWtGGGtGtGG"
    " GtGGWtGGWtG"
    "  GGtWtWtGG "
    "  GGGtGGGtG "
    "   GtGGGtG  "
    "     GGt    "
    // galaxian
    "            "
    "      R     "
    " u   RRR   u"
    " u  RRRRR  u"
    " uYRRYRYRRYu"
    " uYYYYRYYYYu"
    " uuYYYYYYYuu"
    "  uuY Y Yuu "
    "   uu Y uu  "
    "    u Y u   "
    "      Y     "
    "      Y     "
    "            "
    // bell
    "     YY     "
    "   YY  YY   "
    "  YYYYYYYY  "
    "  YY YYYYY  "
    "  Y YYYYYY  "
    " YY YYYYYYY "
    " YY YYYYYYY "
    " YYYYYYYYYY "
    "YY YYYYYYYYY"
    "YY YYYYYYYYY"
    "YYYYYYYYYYYY"
    "YbbbbbWWbbbY"
    " bbbbbWWbbb "
    // key
    "            "
    "     bbb    "
    "   bb   bb  "
    "   bbbbbbb  "
    "   bbbbbbb  "
    "     W W    "
    "     W WW   "
    "     W W    "
    "     W      "
    "     W W    "
    "     W WW   "
    "     W W    "
    "      W     ";
static const int FRUIT_WIDTH = 12;
static const int FRUIT_HEIGHT = 13;
const int MAX_FRUIT = (int)FRUIT_SPRITES.length() / (FRUIT_HEIGHT * FRUIT_WIDTH);

static const std::unordered_map<char, SDL_Color> COLOR_MAP = {
    {'B', COLOR_BROWN},
    {'G', COLOR_GREEN},
    {'R', COLOR_RED},
    {'T', COLOR_TAN},
    {'W', COLOR_WHITE},
    {'Y', COLOR_YELLOW},
    {'b', COLOR_BABY_BLUE},
    {'t', COLOR_TURQUOISE},
    {'u', COLOR_BLUE}};

std::vector<DisplayFruit> DisplayFruit::makeDisplayFruits(GameState& gameState)
{
    int index = 0;
    std::vector<DisplayFruit> displayFruits;
    displayFruits.reserve(MAX_FRUIT);
    for(int i = 0; i < MAX_FRUIT; i++)
    {
        displayFruits.emplace_back(gameState, index++);
    }

    return displayFruits;
}

DisplayFruit::DisplayFruit(GameState& gameState, int index)
: GridObject(gameState, FRUIT_DISPLAY_ROW, FRUIT_DISPLAY_START_COL - index), m_index(index)
{
    m_name = std::string("fruit ") + std::to_string(index);
    m_xPixelOffset = -2 * index;
}

void DisplayFruit::update()
{
    static const int SCALING_FACTOR = 2;

    int spriteIndex = m_index * FRUIT_WIDTH * FRUIT_HEIGHT;
    for(int rowOffset = 0; rowOffset < FRUIT_HEIGHT * SCALING_FACTOR; rowOffset += SCALING_FACTOR)
    {
        for(int colOffset = 0; colOffset < FRUIT_WIDTH * SCALING_FACTOR; colOffset += SCALING_FACTOR)
        {
            for(int rowScale = 0; rowScale < SCALING_FACTOR; rowScale++)
                for(int colScale = 0; colScale < SCALING_FACTOR; colScale++)
                {
                    if(FRUIT_SPRITES[spriteIndex] == ' ')
                    {
                        goto nextPixel;
                    }
                    SDL_Color color = COLOR_MAP.at(FRUIT_SPRITES[spriteIndex]);
                    SDL_SetRenderDrawColor(m_gameState.m_renderer, color.r, color.g, color.b, color.a);
                    SDL_RenderDrawPoint(
                        m_gameState.m_renderer,
                        X_CENTER(m_col) + m_xPixelOffset + colOffset + colScale - FRUIT_WIDTH,
                        Y_CENTER(m_row) + m_yPixelOffset + rowOffset + rowScale - FRUIT_HEIGHT);
                }

nextPixel:
            ++spriteIndex;
        }
    }
}

PointsFruit::PointsFruit(GameState& gameState) : DisplayFruit(gameState, -1)
{
    m_row = FRUIT_SPAWN_ROW;
    m_col = FRUIT_SPAWN_COL;
}

void PointsFruit::update()
{
    if(m_available)
    {
        m_index = m_gameState.m_level - 1;
        if(m_index >= MAX_FRUIT)
        {
            m_index = MAX_FRUIT;
        }
        DisplayFruit::update();
    }
}

void PointsFruit::reset()
{
    m_available = false;
    TimerService::getInstance().stopTimer(m_availabilityTimerKey);
}

void PointsFruit::activate()
{
    m_available = true;
    auto& timerService = TimerService::getInstance();
    m_availabilityTimerKey = timerService.addTimer(FRUIT_DURATION_TICKS, false, [this]() { m_available = false; });
    timerService.startTimer(m_availabilityTimerKey);
}
