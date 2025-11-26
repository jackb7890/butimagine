#include "SDL.h"
#include <array>

bool InputIsUserMovement(const SDL_Event& ev);
std::pair<int, bool> EventGetMovementInfo(SDL_Event ev);
bool InputIsQuitGame(const SDL_Event& ev);