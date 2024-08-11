#pragma once
#include <string>
#include <vector>

// forward declaration
struct SDL_Renderer;

void displayNumber(SDL_Renderer* renderer, int x, int y, int number, SDL_Color color);
void displayString(SDL_Renderer* renderer, int x, int y, const std::string& str, SDL_Color color);
void autoDisplayString(SDL_Renderer* renderer, const std::string& str, SDL_Color color);