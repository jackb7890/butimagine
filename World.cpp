#include "World.hpp"
#include <algorithm>

void MapEntity::Move(int xD, int yD) {
    hasMovedOffScreen = true;
    oldPos = GetCurrentPos();
    GridPos newPos = GridPos(oldPos.x + xD, oldPos.y + yD);
    int smallerXCoord = std::min(oldPos.x, newPos.x);
    int smallerYCoord = std::min(oldPos.y, newPos.y);
    int xCollision = GetWidth() + std::abs(xD);
    int yCollision = GetDepth() + std::abs(yD);
    const HitBox collisionPath = HitBox(smallerXCoord, smallerYCoord, xCollision, yCollision);

    if (map->CheckForCollision(collisionPath, ID)) {
        return;
    }

    if (newPos.x > MAP_WIDTH) {
        newPos.x = oldPos.x % MAP_WIDTH;
    }
    else if (newPos.x < 0) {
        newPos.x = MAP_WIDTH + (oldPos.x % MAP_WIDTH);
    }

    if (newPos.y > MAP_HEIGHT) {
        newPos.y = oldPos.y % MAP_HEIGHT;
    }
    else if (newPos.y < 0) {
        newPos.y = MAP_HEIGHT + (oldPos.y % MAP_HEIGHT);
    }

    map->Clear(*this);
    SetPos(newPos);
    map->Add(*this);
}

void MapEntity::MoveHoriz(int xD) {
    Move(xD, 0);
}

void MapEntity::MoveVert(int yD) {
    Move(0, yD);
}

Map::Map () {
    numberOfEntities = 0;
    grid = Arr2d<MapEntity>(MAP_WIDTH, MAP_HEIGHT);
}

// Drawing each pixel based on each entry of grid for the map
// will be slow compared to if we can do some SDL_FillRects, but
// idk how to we'd do that
void Map::SetStartMap() {
    // random stuff to start it out
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            // grid(i,j) = i*MAP_HEIGHT + j;
            grid(i,j) = MapEntity();
        }
    }
}

// Clears an entity to the map
void Map::Clear(MapEntity entity) {
    for (int i = entity.GetCurrentPos().x; i < entity.GetCurrentPos().x + entity.GetWidth(); i++) {
        for (int j = entity.GetCurrentPos().y; j < entity.GetCurrentPos().y + entity.GetDepth(); j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT) = MapEntity();
        }
    }
}

void Map::Add(Player player) {
    Add((MapEntity) player);
}

void Map::Add(Wall wall) {
    Add((MapEntity) wall);
}

// Adds an entity to the map
void Map::Add(MapEntity entity) {
    for (int i = entity.GetCurrentPos().x; i < entity.GetCurrentPos().x + entity.GetWidth(); i++) {
        for (int j = entity.GetCurrentPos().y; j < entity.GetCurrentPos().y + entity.GetDepth(); j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT) = entity;
        }
    }
}

bool Map::CheckForCollision(const HitBox& movingPiece, int ID)  {
    int xBound = movingPiece.origin.x + movingPiece.dim.width;
    int yBound = movingPiece.origin.y + movingPiece.dim.depth;
    for (int x = movingPiece.origin.x; x < xBound; x++) {
        for (int y = movingPiece.origin.y; y < yBound; y++) {
             MapEntity& possibleEntity = grid(x % MAP_WIDTH, y % MAP_HEIGHT);
            if (possibleEntity.valid && possibleEntity.hasCollision &&
                possibleEntity.ID != ID) {
                return true;
            }
        }
    }
    return false;
}

Display::Display(SDL_Window* _w) : window(_w) {
    surface = SDL_GetWindowSurface(window);
}

Display::~Display() {
    SDL_DestroyWindow(window);
}

void Display::Update(Map map, bool updateScreen) {
    int i, j;
    for (i = 0; i < MAP_WIDTH; i++) {
        for (j = 0; j < MAP_HEIGHT; j++) {
            SDL_Rect point {i, j, 1, 1};
            MapEntity entity = map.grid(i, j);
            if (i == 155 && j == 280) {
                printf("k");
            }
            if (entity.valid) {
                unsigned color = SDL_MapRGB(surface->format, entity.color.r, entity.color.g, entity.color.b);
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

MapEntity::MapEntity(HitBox _hb, RGBColor _c, Map* _map) :
    hitbox(_hb), color(_c), map(_map), hasCollision(true) {
    ID = map->numberOfEntities++;
}

Wall::Wall(GridPos _pos, int _length, bool _isV, RGBColor _c, Map* _map) : 
    MapEntity(HitBox(_pos, GridDimension(thickness, _length)), _c, _map),
    isVert(_isV) {
    // SetWidth(thickness); // fix the place holder
    if (!isVert) {
        std::swap(hitbox.dim.depth, hitbox.dim.width);
    }
}