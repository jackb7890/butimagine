#include "World.hpp"

Player::Player(int _x, int _y, Map& _map) : position(_x, _y), map(_map) {
    map.Add(*this);
}

void Player::MoveHoriz(int xD) {
    hasMovedOffScreen = true;
    oldPos = position;
    map.Clear(*this);
    position.x += xD;
    if (position.x > MAP_WIDTH) {
        position.x = position.x % MAP_WIDTH;
    }
    map.Add(*this);
}

void Player::MoveVert(int yD) {
    hasMovedOffScreen = true;
    oldPos = position;
    map.Clear(*this);
    position.y += yD;
    if (position.y > MAP_HEIGHT) {
        position.y = position.y % MAP_HEIGHT;
    }
    map.Add(*this);
}

Map::Map () {
    grid = Arr2d<int>(MAP_WIDTH, MAP_HEIGHT);
}

// Drawing each pixel based on each entry of grid for the map
// will be slow compared to if we can do some SDL_FillRects, but
// idk how to we'd do that
void Map::SetStartMap() {
    // random stuff to start it out
    const int stride = 5;
    for (int i = 0; i < MAP_WIDTH; i+=stride) {
        for (int j = 0; j < MAP_HEIGHT; j+=stride) {
            grid(i,j) = i*MAP_HEIGHT + j;
        }
    }
}

// Clears the map at area covered by player
void Map::Clear(Player player) {
    for (int i = player.position.x; i < player.position.x + player.width; i++) {
        for (int j = player.position.y; j < player.position.y + player.height; j++) {
            grid(i % MAP_WIDTH,j % MAP_HEIGHT) = 0;
        }
    }
}

// Adds a player to the map
void Map::Add(Player player) {
    for (int i = player.position.x; i < player.position.x + player.width; i++) {
        for (int j = player.position.y; j < player.position.y + player.height; j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT) = player.playerID;
        }
    }
}

// Adds a wall to the map
void Map::Add(Wall wall) {
    int width = wall.width;
    int length = wall.length;
    if (!wall.isVert) std::swap(width, length);
    for (int i = wall.origin.x; i < wall.origin.x + width; i++) {
        for (int j = wall.origin.y; j < wall.origin.y + length; j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT) = wall.color.r;
        }
    }
}

Display::Display(SDL_Window* _w) : window(_w) {
    surface = SDL_GetWindowSurface(window);
}

Display::~Display() {
    SDL_DestroyWindow(window);
}

void Display::Update(Map map, bool updateScreen) {
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            SDL_Rect point {i, j, 1, 1};
            SDL_FillRect(surface, &point, map.grid(i,j));
        }
    }
    if (updateScreen) {
        SDL_UpdateWindowSurface(window);
    }
}

void Display::Erase(Player player, bool updateScreen) {
    SDL_Rect blankRect = {player.oldPos.x, player.oldPos.y, player.width, player.height};
    SDL_FillRect(surface, &blankRect, 0);
    if (updateScreen) {
        SDL_UpdateWindowSurface(window);
    }
}

void Display::Update(Player player, bool updateScreen) {
    if (player.hasMovedOffScreen) {
        Erase(player, false /* false so we don't update in Erase and update again here*/);
    }
    SDL_Rect playerRect = SDL_Rect {player.position.x, player.position.y, player.width, player.height};
    SDL_FillRect(surface, &playerRect, player.playerID);
    player.hasMovedOffScreen = false;  // we just drew it, so it hasn't moved from what's on the screen for now
    if (updateScreen) {
        SDL_UpdateWindowSurface(window);
    }
}

void Display::Update(Wall wall, bool updateScreen) {
    SDL_Rect wallRect = wall.isVert ? 
        SDL_Rect {wall.origin.x, wall.origin.y, wall.width, wall.length} :
        SDL_Rect {wall.origin.x, wall.origin.y, wall.length, wall.width};
    SDL_FillRect(surface, &wallRect, SDL_MapRGB(surface->format, wall.color.r, wall.color.g, wall.color.b));
    if (updateScreen) {
        SDL_UpdateWindowSurface(window);
    }
}