#include <SDL.h>

#include "GameState.hpp"
#include "font.hpp"
#include "util.hpp"

GameState::GameState(SDL_Renderer* renderer) : m_renderer(renderer)
{
    LOG_INFO("Constructing GameState");
    m_ghostSpawnTimer.start();
    m_readyTimer.start();
}

void GameState::update()
{
    // handle moving to next level
    if(m_dotsRemaining <= 0)
    {
        m_level++;
        m_board = BASE_LAYOUT;
        LOG_INFO("Level: %d", m_level);
    }

    if(m_score >= m_extraLifeThreshold)
    {
        m_lives++;
        m_extraLifeThreshold += m_extraLivesIncrement;
        LOG_INFO("Earned extra life, lives %d", m_lives);
    }

    // draw stationary elements
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(m_renderer);
    drawScore();
    drawFullBoard();

    for(size_t levelFruitIndex = 0; levelFruitIndex < m_level; levelFruitIndex++)
    {
        size_t index = levelFruitIndex >= m_displayFruits.size() ? (m_displayFruits.size() - 1) : levelFruitIndex;
        m_displayFruits[index].update();
    }

    if(gameOver())
    {
        displayString(m_renderer, 720 / 2 - (16 * 9) / 2, 550, "GAME OVER", COLOR_YELLOW);
        goto finishRender;
    }

    for(auto& ghost : m_ghosts)
    {
        if(m_pacman.hasSamePositionAs(ghost))
        {
            if(ghost.m_isFlashing)
            {
                m_score += m_flashingGhostPoints;
                m_flashingGhostPoints *= 2;
                ghost.reset();
            }
            else
            {
                LOG_INFO("Found a ghost, lose a life: %d -> %d", m_lives, m_lives - 1);
                m_lives--;

                m_pacman.reset();
                for(auto& ghost : m_ghosts)
                {
                    ghost.reset();
                }
            }
        }
    }

    m_ghostSpawnTimer.check();
    m_flashingGhostTimer.check();
    m_readyTimer.check();

    if(m_readyDisplayed)
    {
        displayString(m_renderer, 720 / 2 - (16 * 5) / 2, 550, "READY", COLOR_YELLOW);
    }

    static const int LIFE_DISPLAY_PADDING = 10;

    for(int displayLife = 0; displayLife < m_lives; displayLife++)
    {
        Pacman::drawPacman(
            m_renderer,
            X_CENTER(1 + displayLife) + LIFE_DISPLAY_PADDING * displayLife,
            Y_CENTER(31),
            Direction::LEFT,
            Pacman::RADIUS);
    }

    // draw points claimable fruit if it is active
    m_fruit.update();

    // draw moving elements
    if(m_activePlay)
    {
        m_pacman.update();
    }

    for(auto& ghost : m_ghosts)
    {
        ghost.update();
    }

finishRender:
    SDL_RenderPresent(m_renderer);
}

void GameState::handleKeypress(const SDL_Keycode keyCode)
{
    switch(keyCode)
    {
    case SDLK_UP:
        m_pacman.changeDirection(Direction::UP);
        break;
    case SDLK_DOWN:
        m_pacman.changeDirection(Direction::DOWN);
        break;
    case SDLK_LEFT:
        m_pacman.changeDirection(Direction::LEFT);
        break;
    case SDLK_RIGHT:
        m_pacman.changeDirection(Direction::RIGHT);
        break;
    default:
        LOG_WARN("Unsupported keypress %d", keyCode);
    }
}

bool GameState::gameOver()
{
    return m_lives <= 0;
}

void GameState::handlePacmanArrival()
{
    // if a key is being held down, attempt to handle it
    // the main loop handles the direction change if pacman is stopped
    int numKeys = 0;
    const uint8_t* const keys = SDL_GetKeyboardState(&numKeys);
    if(numKeys > 0)
    {
        if(keys[SDL_SCANCODE_UP])
            m_pacman.changeDirection(Direction::UP);
        else if(keys[SDL_SCANCODE_DOWN])
            m_pacman.changeDirection(Direction::DOWN);
        else if(keys[SDL_SCANCODE_LEFT])
            m_pacman.changeDirection(Direction::LEFT);
        else if(keys[SDL_SCANCODE_RIGHT])
            m_pacman.changeDirection(Direction::RIGHT);
    }

    if(m_fruit.isActive() && m_pacman.hasSamePositionAs(m_fruit))
    {
        m_score += m_fruitPoints;
        m_fruitPoints *= m_fruitPointsMultiplier;
        m_fruit.reset();
    }

    auto [row, col] = m_pacman.getPosition();
    auto& pacmansTile = m_board[row][col];
    switch(pacmansTile)
    {
    case DOT:
        m_score += m_normalDotPoints;
        m_dotsEaten++;
        pacmansTile = ' ';
        break;
    case SUPER_DOT:
        m_score += m_superDotPoints;
        pacmansTile = ' ';
        for(auto& ghost : m_ghosts)
        {
            ghost.handleSuperDot();
            m_flashingGhostTimer.start();
        }
        break;
    case WRAP:
        m_pacman.relocate(row, (int)m_board[0].size() - col - 1);
        break;
    default:
        return;
    }

    if(m_dotsEaten >= m_fruitThreshold)
    {
        m_fruit.activate();
        m_fruitThreshold += m_fruitThresholdIncrement;
    }

    LOG_INFO("Score: %d", m_score);
}

void GameState::drawScore()
{
    displayString(m_renderer, 150, 6, "1 UP", COLOR_TURQUOISE);
    displayNumber(m_renderer, 150 + 4 * 16, 24, m_score, COLOR_WHITE);
}

void GameState::drawFullBoard()
{
    m_dotsRemaining = 0;
    SDL_SetRenderDrawColor(m_renderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
    for(int row = 0; row < (int)m_board.size(); row++)
    {
        for(int col = 0; col < (int)m_board[row].size(); col++)
        {
            int rowCenter = Y_CENTER(row);
            int colCenter = X_CENTER(col);
            switch(m_board[row][col])
            {
            case DOT:
                drawFilledCircle(m_renderer, colCenter, rowCenter, 4, {0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE});
                m_dotsRemaining++;
                break;
            case SUPER_DOT:
                m_dotsRemaining++;
                drawFilledCircle(m_renderer, colCenter, rowCenter, 8, {0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE});
                break;
            case BOUNDARY:
                drawBoundary(row, col);
                break;
            default:
                break;
            }
        }
    }
}

void GameState::drawBoundary(int row, int col)
{
    std::vector<Direction> edges;

    for(size_t dir = 0; dir < (size_t)Direction::MAX; dir++)
    {
        int adjRow = row + Y_INCREMENT[dir];
        int adjCol = col + X_INCREMENT[dir];
        if(adjRow < 0 || adjRow >= m_board.size() || adjCol < 0 || adjCol >= m_board[adjRow].size())
        {
            continue;
        }

        if(m_board[adjRow][adjCol] == BOUNDARY)
        {
            edges.push_back((Direction)dir);
        }
    }

    for(Direction dir : edges)
    {
        int xInc = X_INCREMENT[(size_t)dir];
        int yInc = Y_INCREMENT[(size_t)dir];
        SDL_RenderDrawLine(m_renderer, X_CENTER(col + xInc), Y_CENTER(row + yInc), X_CENTER(col), Y_CENTER(row));
    }
}
