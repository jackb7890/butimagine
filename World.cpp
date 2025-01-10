#include "World.hpp"

void MapEntity::MoveHoriz(int xD) {
    hasMovedOffScreen = true;
    oldPos = GetCurrentPos();
    int newX = oldPos.x+xD;

    if (newX > MAP_WIDTH) {
        newX = oldPos.x % MAP_WIDTH;
    }
    else if (newX < 0) {
        newX = MAP_HEIGHT + (oldPos.x % MAP_HEIGHT);
    }

    map.Clear(this);
    SetPos(newX, oldPos.y);
    map.Add(this);
}

void MapEntity::MoveVert(int yD) {
    hasMovedOffScreen = true;
    oldPos = GetCurrentPos();
    int newY = oldPos.y+yD;

    if (newY > MAP_HEIGHT) {
        newY = oldPos.y % MAP_HEIGHT;
    }
    else if (newY < 0) {
        newY = MAP_WIDTH + (oldPos.y % MAP_WIDTH);
    }

    map.Clear(this);
    SetPos(oldPos.x, newY);
    map.Add(this);
}

Map::Map () {
    grid = Arr2d<MapEntity*>(MAP_WIDTH, MAP_HEIGHT);
}

// Drawing each pixel based on each entry of grid for the map
// will be slow compared to if we can do some SDL_FillRects, but
// idk how to we'd do that
void Map::SetStartMap() {
    // random stuff to start it out
    const int stride = 5;
    for (int i = 0; i < MAP_WIDTH; i+=stride) {
        for (int j = 0; j < MAP_HEIGHT; j+=stride) {
            // grid(i,j) = i*MAP_HEIGHT + j;
            grid(i,j) = nullptr;
        }
    }
}

// Clears an entity to the map
void Map::Clear(MapEntity* entity) {
    for (int i = entity->GetCurrentPos().x; i < entity->GetCurrentPos().x + entity->GetWidth(); i++) {
        for (int j = entity->GetCurrentPos().y; j < entity->GetCurrentPos().y + entity->GetDepth(); j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT) = nullptr;
        }
    }
}

// Adds an entity to the map
void Map::Add(MapEntity* entity) {
    for (int i = entity->GetCurrentPos().x; i < entity->GetCurrentPos().x + entity->GetWidth(); i++) {
        for (int j = entity->GetCurrentPos().y; j < entity->GetCurrentPos().y + entity->GetDepth(); j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT) = entity;
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
            MapEntity* entity = map.grid(i, j);
            if (entity) {
                unsigned color = SDL_MapRGB(surface->format, entity->color.r, entity->color.g, entity->color.b);
                SDL_FillRect(surface, &point, color);
            }
            else {
                SDL_FillRect(surface, &point, i*MAP_HEIGHT + j);
            }
        }
    }
    if (updateScreen) {
        SDL_UpdateWindowSurface(window);
    }
}

void Display::Erase(Player player, bool updateScreen) {
    SDL_Rect blankRect = {player.oldPos.x, player.oldPos.y, player.GetWidth(), player.GetDepth()};
    SDL_FillRect(surface, &blankRect, 0);
    if (updateScreen) {
        SDL_UpdateWindowSurface(window);
    }
}

void Display::Update(Player player, bool updateScreen) {
    if (player.hasMovedOffScreen) {
        Erase(player, false /* false so we don't update in Erase and update again here*/);
    }
    Update((MapEntity) player, updateScreen);
    player.hasMovedOffScreen = false;  // we just drew it, so it hasn't moved from what's on the screen for now
    if (updateScreen) {
        SDL_UpdateWindowSurface(window);
    }
}

void Display::Update(Wall wall, bool updateScreen) {
    Update((MapEntity) wall, updateScreen);
}

void Display::Update(MapEntity entity, bool updateScreen) {
    SDL_Rect rect = entity.GetSDLRect();
    unsigned color = SDL_MapRGB(surface->format, entity.color.r, entity.color.g, entity.color.b);
    SDL_FillRect(surface, &rect, color);
    if (updateScreen) {
        SDL_UpdateWindowSurface(window);
    }
}

MapEntity::MapEntity(HitBox _hb, RGBColor _c, Map& _map) :
    hitbox(_hb), color(_c), map(_map), hasCollision(true) {
    map.Add(this);
}

Wall::Wall(GridPos _pos, int _length, bool _isV, RGBColor _c, Map& _map) : 
    MapEntity(HitBox(_pos, GridDimension(-1 /* placeholder because thickness isn't initazlied*/, _length)), _c, _map),
    isVert(_isV) {
    SetWidth(thickness); // fix the place holder
    if (!isVert) {
        std::swap(hitbox.dim.depth, hitbox.dim.width);
    }
}