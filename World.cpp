#include "World.hpp"
#include "util.hpp"
#include <algorithm>
#include <type_traits>
#include <functional>

unsigned RGBColor::ConvertToSDL(SDL_Surface* surface) {
    return SDL_MapRGB(surface->format, r, g, b);
}

std::string MapEntity::ToString() {
    char buf[256];

    sprintf_s(buf, 256, "ID: %zd, rgb: %d:%d:%d, location: (%d, %d), size(XxY): %dx%d",
        this->ID, this->color.r, this->color.g, this->color.b, this->hitbox.origin.x, 
        this->hitbox.origin.y, this->hitbox.dim.width, this->hitbox.dim.depth);

    return std::string((char*)buf);
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
    Log logger("MapEntity Move: ");
    logger.Emit("entity=%d, attempting to move (xD=%d, yD=%d)\n", this->ID, xD, yD);

    if (!xD && !yD) {
        logger.Emit("failed to move, called with (0,0)\n");
        return;
    }

    hasMovedOffScreen = true;
    HitBox oldHb = this->hitbox;
    oldPos = this->GetCurrentPos();

    logger.Emit("prior location: (%d, %d)\n", oldPos.x, oldPos.y);

    GridPos newPos = GridPos(oldPos.x + xD, oldPos.y + yD);

    int smallerXCoord = std::min(oldPos.x, newPos.x);
    int smallerYCoord = std::min(oldPos.y, newPos.y);
    int xCollision = GetWidth() + std::abs(xD);
    int yCollision = GetDepth() + std::abs(yD);
    const HitBox collisionPath = HitBox(smallerXCoord, smallerYCoord, xCollision, yCollision);

    if (map->CheckForCollision(collisionPath, ID)) {
        logger.Emit("failed to move, collision encountered\n");
        return;
    }

    newPos.x = Wrap(oldPos.x, xD, MAP_WIDTH);
    newPos.y = Wrap(oldPos.y, yD, MAP_HEIGHT);

    logger.Emit("new location: (%d, %d)\n", newPos.x, newPos.y);

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

Display::Display(Map* _map) : map(_map) {
    surface = nullptr;
}

Display::~Display() {
    SDL_DestroyWindow(window);
}

void Display::DrawEntity(MapEntity entity) {
    Log::emit("Drawing Entity...\n");
    Log::emit("\tEntity description -> %s\n", entity.ToString().c_str());

    SDL_Rect point = SDL_Rect {
        entity.hitbox.origin.x,
        entity.hitbox.origin.y,
        entity.hitbox.dim.width,
        entity.hitbox.dim.depth
    };

    //A- Unlike SDL_FillRect from the window based rendering,
    //A- RenderFillRect doesn't have a color input.
    //A- Color is set beforehand by SetRenderDrawColor
    //A- It's also one field for each RGB, so use (map)entity.color.(r/g/b) instead of RGBColor.ConvertToSDL()

    SDL_SetRenderDrawColor(this->renderer, entity.color.r, entity.color.g, entity.color.b, 255);
    SDL_RenderFillRect(this->renderer, &point);
}

void Display::DrawBackground() {
    Log::emit("Drawing Background...\n");

    SDL_Rect rect = SDL_Rect { 0, 0, map->width, map->height };
    RGBColor color = RGBColor { 0, 0, 0 };

    SDL_SetRenderDrawColor(this->renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(this->renderer, &rect);
}

void Display::DrawFrame(std::vector<MapEntity*> entities) {

    Log::emit("Clear Renderer\n");
    SDL_RenderClear(this->renderer);

    DrawBackground();

    for (auto entity : entities) {
        DrawEntity(*entity);
    }

    Log::emit("Present Renderer\n");
    SDL_RenderPresent(this->renderer);
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