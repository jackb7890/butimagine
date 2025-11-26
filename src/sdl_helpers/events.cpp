#include "events.hpp"
bool InputIsUserMovement(const SDL_Event& ev) {
    if (ev.type != SDL_KEYDOWN && ev.type != SDL_KEYUP) {
        return false;
    }

    switch (ev.key.keysym.sym) {
        case SDLK_w:
        case SDLK_a:
        case SDLK_s:
        case SDLK_d:
        return true;
        
        default:
        return false;
    }
    return false;
}

std::pair<int, bool> EventGetMovementInfo(SDL_Event ev) {
    return std::pair(ev.key.keysym.sym, ev.type == SDL_KEYUP);
}

bool InputIsQuitGame(const SDL_Event& ev) {
    return ev.type == SDL_QUIT;
}