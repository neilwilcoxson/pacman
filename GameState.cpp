#include <SDL.h>

#include "GameState.hpp"
#include "util.hpp"

GameState::GameState(SDL_Renderer* renderer) : m_renderer(renderer)
{
    m_ghosts.reserve(4);
    m_ghosts.emplace_back(15, 13, Direction::LEFT, COLOR_RED, "RED");
    m_ghosts.emplace_back(15, 14, Direction::LEFT, COLOR_GREEN, "GREEN");
    m_ghosts.emplace_back(15, 15, Direction::LEFT, COLOR_BLUE, "BLUE");
}

void GameState::draw()
{
    // draw stationary elements
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(m_renderer);

    drawFullBoard(m_renderer, m_board);

    m_pacman.draw(m_renderer, m_board);

    uint64_t currentTicks = SDL_GetTicks64();
    if (currentTicks > m_nextGhostTicks)
    {
        for (auto& ghost : m_ghosts)
        {
            if (ghost.inBox)
            {
                const int GHOST_SPAWN_ROW = 11;
                const int GHOST_SPAWN_COL = 15;
                ghost.relocate(GHOST_SPAWN_ROW, GHOST_SPAWN_COL);
                ghost.inBox = false;
                m_nextGhostTicks = currentTicks + m_ghostSpwanIntervalTicks;
                break;
            }
        }

    }
    for (auto& ghost : m_ghosts)
    {
        ghost.draw(m_renderer, m_board);
    }
    SDL_RenderPresent(m_renderer);
}

void GameState::handleKeypress(const SDL_KeyCode keyCode)
{
    switch (keyCode)
    {
    case SDLK_UP:
        m_pacman.changeDirection(m_board, Direction::UP);
        break;
    case SDLK_DOWN:
        m_pacman.changeDirection(m_board, Direction::DOWN);
        break;
    case SDLK_LEFT:
        m_pacman.changeDirection(m_board, Direction::LEFT);
        break;
    case SDLK_RIGHT:
        m_pacman.changeDirection(m_board, Direction::RIGHT);
        break;
    default:
        LOG_WARN("Unsupported keypress %d", keyCode);
    }
}

void drawFullBoard(SDL_Renderer* renderer, BoardLayout& board)
{
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
    for (size_t row = 0; row < board.size(); row++)
    {
        for (size_t col = 0; col < board[row].size(); col++)
        {
            int rowCenter = Y_CENTER(row);
            int colCenter = X_CENTER(col);
            switch (board[row][col])
            {
            case '.':
            {
                drawFilledCircle(renderer, colCenter, rowCenter, 4, { 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE });
                break;
            }
            case '*':
            {
                drawFilledCircle(renderer, colCenter, rowCenter, 8, { 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE });
                break;
            }
            case 'x':
            {
                // TODO cleanup board edges
                if (board[row - 1][col] == 'x' && board[row + 1][col] == 'x')
                {
                    // pure vertical
                    SDL_RenderDrawLine(renderer, colCenter, row * TILE_HEIGHT, colCenter, (row + 1) * TILE_HEIGHT);
                }
                else if (board[row][col - 1] == 'x' && board[row][col + 1] == 'x')
                {
                    // pure horizontal
                    SDL_RenderDrawLine(renderer, col * TILE_WIDTH, rowCenter, (col + 1) * TILE_WIDTH, rowCenter);
                }
                else
                {
                    std::vector<std::pair<int, int>> points;
                    auto pushIfX = [&board, &points](int x, int y)
                    {
                        if (board[y][x] == 'x') { points.push_back({ x, y }); }
                    };
                    pushIfX(col, row - 1);
                    pushIfX(col, row + 1);
                    pushIfX(col - 1, row);
                    pushIfX(col + 1, row);

                    if (points.size() == 2)
                    {
                        // connector
                        SDL_RenderDrawLine(renderer, X_CENTER(points[0].first), Y_CENTER(points[0].second), colCenter, rowCenter);
                        SDL_RenderDrawLine(renderer, colCenter, rowCenter, X_CENTER(points[1].first), Y_CENTER(points[1].second));

                    }
                    else
                    {
                        LOG_TRACE("Attempted connection with size=%llu at row=%llu col=%llu", points.size(), row, col);
                    }
                }
                break;
            }
            default:
            {
                break;
            }
            }
        }
    }
}