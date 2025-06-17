#include "World.hpp"
#include <algorithm>
#include <type_traits>
#include <functional>

unsigned RGBColor::ConvertToSDL(SDL_Surface* surface) {
    return SDL_MapRGB(surface->format, r, g, b);
}

void MapEntity::UpdateGrid(HitBox oldArea, HitBox newArea) {
    for (int i = oldArea.origin.x; i < oldArea.origin.x + oldArea.dim.width; i++) {
        for (int j = oldArea.origin.y; j < oldArea.origin.y + oldArea.dim.depth; j++) {
            map->grid(i % MAP_WIDTH, j % MAP_HEIGHT).Remove(this);
        }
    }

    for (int i = newArea.origin.x; i < newArea.origin.x + newArea.dim.width; i++) {
        for (int j = newArea.origin.y; j < newArea.origin.y + newArea.dim.depth; j++) {
            map->grid(i % MAP_WIDTH, j % MAP_HEIGHT).Add(this);
        }
    }
}

void MapEntity::Move(int xD, int yD) {
    hasMovedOffScreen = true;
    HitBox oldHb = this->hitbox;
    oldPos = this->GetCurrentPos();
    GridPos newPos = GridPos(oldPos.x + xD, oldPos.y + yD);
    int smallerXCoord = std::min(oldPos.x, newPos.x);
    int smallerYCoord = std::min(oldPos.y, newPos.y);
    int xCollision = GetWidth() + std::abs(xD);
    int yCollision = GetDepth() + std::abs(yD);
    const HitBox collisionPath = HitBox(smallerXCoord, smallerYCoord, xCollision, yCollision);

    if (map->CheckForCollision(collisionPath, ID)) {
        return;
    }

    newPos.x = Wrap(oldPos.x, xD, MAP_WIDTH);
    newPos.y = Wrap(oldPos.y, yD, MAP_HEIGHT);

    this->SetPos(newPos);

    // Update grid structure
    HitBox newHb = this->hitbox;
    this->UpdateGrid(oldHb, newHb);
}

void MapEntity::Move(int xD, int yD) {
        hasMovedOffScreen = true;
        oldPos = GetCurrentPos();
        GridPos newPos = GridPos(oldPos.x + xD, oldPos.y + yD);

        int smallerXCoord = std::min(oldPos.x, newPos.x);
        int smallerYCoord = std::min(oldPos.y, newPos.y);
        int xCollision = GetWidth() + std::abs(xD);
        int yCollision = GetDepth() + std::abs(yD);
        const HitBox collisionPath = HitBox(smallerXCoord, smallerYCoord, xCollision, yCollision);

        if (this->hasCollision) {
           if (map->CheckForCollision(collisionPath, this->ID)) {
                //A- Collisions set velocity to 0. Makes bumping into walls less annoying
                this->X_velocity = 0.0;
                this->Y_velocity = 0.0;
                return;
            }
        }

        newPos.x = Wrap(oldPos.x, xD, MAP_WIDTH);
        newPos.y = Wrap(oldPos.y, yD, MAP_HEIGHT);

        SetPos(newPos);
}

Map::Map () {
    numberOfEntities = 0;
    grid = Arr2d<MapEntityList>(MAP_WIDTH, MAP_HEIGHT);
}

Map::~Map () {
    for (MapEntity* entity : allEntities) {
        delete entity;
    }
}

bool Map::CheckForCollision(const HitBox& movingPiece, size_t ID)  {
    int xBound = movingPiece.origin.x + movingPiece.dim.width;
    int yBound = movingPiece.origin.y + movingPiece.dim.depth;
    for (int x = movingPiece.origin.x; x < xBound; x++) {
        for (int y = movingPiece.origin.y; y < yBound; y++) {
                        
            MapEntityList entitiesAtPoint = grid(Wrap(x-1, 1, MAP_WIDTH), Wrap(y - 1, 1, MAP_HEIGHT));
            for (MapEntity* entity : entitiesAtPoint.list) {
                if (entity->valid && entity->hasCollision &&
                    entity->ID != ID) {
                    return true;
                }
            }
        }
    }
    return false;
}

MapEntity* Map::GetEntity(size_t id) {
    for (auto entity : allEntities) {
        if (entity->ID == id) {
            return entity;
        } 
    }
    return nullptr;
}

Display::Display(SDL_Window* _w, Map* _map) : window(_w), map(_map) {
    surface = SDL_GetWindowSurface(window);
}

Display::~Display() {
    SDL_DestroyWindow(window);
}

void Display::DrawBackground() {
    SDL_Rect bg = {0, 0, MAP_WIDTH, MAP_HEIGHT};
    SDL_FillRect(surface, &bg, 0);
}

void Display::Publish() {
    SDL_UpdateWindowSurface(window);
}

void Display::DrawEntity(MapEntity* entity) {
    SDL_Rect rect = entity->GetSDLRect();
    SDL_FillRect(surface, &rect, entity->color.ConvertToSDL(surface));
}

void Display::DrawEntities(std::vector<MapEntity*> entities) {
    for (auto entity : entities) {
        Display::DrawEntity(entity);
    }
    Display::Publish();
}

void Display::PublishNextFrame(std::vector<MapEntity*> entities) {
    Display::DrawBackground();
    Display::DrawEntities(entities);
    Display::Publish();
}

MapEntity::MapEntity(HitBox _hb, RGBColor _c, Map* _map, bool _hasCol) :
    hitbox(_hb), color(_c), map(_map), hasCollision(_hasCol) {
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
