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

void MapEntity::MoveHoriz(int xD) {
    Move(xD, 0);
}

void MapEntity::MoveVert(int yD) {
    Move(0, yD);
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

//A- Renderer added to display struct
Display::Display(SDL_Window* _w, SDL_Renderer* _r, Map* _map) : window(_w), renderer(_r), map(_map) {
    //A- Surface isn't used anymore, can probably remove
    surface = SDL_GetWindowSurface(window);
}

Display::Display(SDL_Window* _w, Map* _map) : window(_w), renderer(nullptr), map(_map) {
    //A- Surface isn't used anymore, can probably remove
    surface = SDL_GetWindowSurface(window);
}

Display::~Display() {
    SDL_DestroyWindow(window);
}

void Display::DrawEntity(MapEntity entity) {
    SDL_Rect point = SDL_Rect {
        entity.hitbox.origin.x,
        entity.hitbox.origin.y,
        entity.hitbox.dim.width,
        entity.hitbox.dim.depth
    };

    if (entity.valid) {
        //A- Unlike SDL_FillRect from the window based rendering,
        //A- RenderFillRect doesn't have a color input.
        //A- Color is set beforehand by SetRenderDrawColor
        //A- It's also one field for each RGB, so use (map)entity.color.(r/g/b) instead of RGBColor.ConvertToSDL()
        
        
        // SDL_SetRenderDrawColor(renderer, entity.color.r, entity.color.g, entity.color.b, 255);
        // SDL_RenderFillRect(renderer, &point);
    }
}

void Display::DrawBackground() {
    SDL_Rect rect = SDL_Rect { 0, 0, map->width, map->height };
    RGBColor color = RGBColor { 0, 0, 0 };

    // SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    // SDL_RenderFillRect(renderer, &rect);
}

void Display::DrawFrame(std::vector<MapEntity*> entities) {

    DrawBackground();

    for (auto entity : entities) {
        Display::DrawEntity(*entity);
    }
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