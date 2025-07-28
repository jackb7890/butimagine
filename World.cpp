#include "World.hpp"
#include <algorithm>

void Tile::PushEntityToList(MapEntity* entitiy) {
    entitiesInTile.push_back(entity);
}

unsigned RGBColor::ConvertToSDL(SDL_Surface* surface) {
    return SDL_MapRGB(surface->format, r, g, b);
}

//A- Just checks if a map entity has a map.
bool MapEntity::Valid() {
    if (this->map) {
        return true;
    }
    return false;
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
    //A- Map_width & height are the same as the window width & height currently
    grid = Arr2d<Tile>(MAP_WIDTH, MAP_HEIGHT);
}

void Map::AddToGrid(MapEntity* entity) {
    for (int i = entity.GetCurrentPos().x; i < entity.GetCurrentPos().x + entity.GetWidth(); i++) {
        for (int j = entity.GetCurrentPos().y; j < entity.GetCurrentPos().y + entity.GetDepth(); j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT).PushEntityToList(entity);
        }
    }
}

void Map::AddEntity(MapEntity* entity) {
    entity->map = this;
    //A- Conveniently the size of the vector before an entity is added will equal its index.
    entity->ID = allEntities.size();
    allEntities.push_back(entity);
    AddToGrid(entity);
}

//A- In the future we may want special behavior for adding players.
void Map::AddEntity(Player* player) {
    AddEntity((MapEntity*)player);
}
void Map::AddEntity(Wall* wall) {
    AddEntity((MapEntity*)wall);
}

bool Map::CheckForCollision(const HitBox& movingPiece, int ID) {
    int xBound = movingPiece.origin.x + movingPiece.dim.width;
    int yBound = movingPiece.origin.y + movingPiece.dim.depth;
        for (int x = movingPiece.origin.x; x < xBound; x++) {
            for (int y = movingPiece.origin.y; y < yBound; y++) {
                std::list<MapEntity*> entitiesAtXYPos = grid(Wrap(x - 1, 1, MAP_WIDTH), Wrap(y - 1, 1, MAP_HEIGHT)).entitiesInTile;
                for (auto entity : entitiesAtXYPos) {
                    if (entity->Valid() && entity->hasCollision && entity->ID != ID) {
                        return true;
                    }   
                }
            }
        }
    return false;
}

Display::Display(SDL_Window* _w, SDL_Renderer* _r, Map* _map) : window(_w), renderer(_r), map(_map) {}

Display::~Display() {
    SDL_DestroyWindow(window);
}

void Display::Update() {
    //This will always draw & render all entities 1 by 1, even if they haven't changed in any way.
    //Isn't the most optimal thing but an easy enough fix later
    //Would just need some kind of "updated" flag in MapEntity (or Map).

    for (MapEntity* entity : map->allEntities) {
        if (entity->Valid()) {
            SDL_SetRenderDrawColor(renderer, entity->color.r, entity->color.g, entity->color.b, 255);
            SDL_Rect point{entity->GetSDLRect()};
            SDL_RenderFillRect(renderer, &point);
        }
    }
    Render();
}

void Display::Render() {
    SDL_RenderPresent(renderer);
}

Wall::Wall(GridPos _pos, int _length, bool _isV, RGBColor _c) : 
    MapEntity(HitBox(_pos, GridDimension(thickness, _length)), _c),
    isVert(_isV) {
    // SetWidth(thickness); // fix the place holder
    if (!isVert) {
        std::swap(hitbox.dim.depth, hitbox.dim.width);
    }
}
