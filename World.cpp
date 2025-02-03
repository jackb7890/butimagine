#include "World.hpp"
#include <algorithm>

unsigned RGBColor::ConvertToSDL(SDL_Surface* surface) {
    return SDL_MapRGB(surface->format, r, g, b);
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

    //A- I added a check to see if the object that is moving has collision.
    //A- Ideally both objects need to have collision on for them to collide.
    if (this->hasCollision == true) {
        if (map->CheckForCollision(collisionPath, ID)) {
            //A- Collisions set velocity to 0. Makes bumping into walls less annoying
            this->X_velocity = 0.0;
            this->Y_velocity = 0.0;
            return;
        }
    }

    newPos.x = Wrap(oldPos.x, xD, MAP_WIDTH);
    newPos.y = Wrap(oldPos.y, yD, MAP_HEIGHT);

    map->Clear(*this);
    SetPos(newPos);
    map->Add(*this);
}

//A- I get intrusive thoughts and add forced movement despite having 0 use for it yet.
void MapEntity::ForceMove(int xD, int yD) {
    //A- If the object *does* have collision, this just turns it off temporarily to move it, then turns it back on.
    if (this->hasCollision == true) {
        this->hasCollision = false;
        Move(xD, yD);
        this->hasCollision = true;
    }
    else Move(xD, yD);
}

Map::Map () {
    numberOfEntities = 0;
    grid = Arr2d<MapObject>(MAP_WIDTH, MAP_HEIGHT);
    background = Arr2d<MapObject>(MAP_WIDTH, MAP_HEIGHT);
}

// Drawing each pixel based on each entry of grid for the map
// will be slow compared to if we can do some SDL_FillRects, but
// idk how to we'd do that
void Map::CreateBackground() {
    //A- I changed the i and j to x and y, to make it a bit less confusing
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            HitBox hb = HitBox(x, y, 1, 1);
            int index = x*MAP_HEIGHT+y;
            RGBColor color = RGBColor(index % 255 / 2, (1 / (index+1)) % 255, (index / 10000) % 255);
            background(x,y) = MapObject(hb, color, this, false /* don't give background collision */);
        }
    }
}

// Clears an object on the map
void Map::Clear(MapObject entity) {
    for (int x = entity.GetCurrentPos().x; x < entity.GetCurrentPos().x + entity.GetWidth(); x++) {
        for (int y = entity.GetCurrentPos().y; y < entity.GetCurrentPos().y + entity.GetDepth(); y++) {
            grid(x % MAP_WIDTH, y % MAP_HEIGHT) = background(x % MAP_WIDTH, y % MAP_HEIGHT);
        }
    }
}

void Map::Add(Player player) {
    Add((MapObject) player);
}

void Map::Add(Wall wall) {
    Add((MapObject) wall);
}

// Adds an object to the map
void Map::Add(MapObject object) {
    for (int i = object.GetCurrentPos().x; i < object.GetCurrentPos().x + object.GetWidth(); i++) {
        for (int j = object.GetCurrentPos().y; j < object.GetCurrentPos().y + object.GetDepth(); j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT) = object;
        }
    }
}

bool Map::CheckForCollision(const HitBox& movingPiece, int ID)  {
    int xBound = movingPiece.origin.x + movingPiece.dim.width;
    int yBound = movingPiece.origin.y + movingPiece.dim.depth;
    for (int x = movingPiece.origin.x; x < xBound; x++) {
        for (int y = movingPiece.origin.y; y < yBound; y++) {
                        
            MapObject& possibleEntity = grid(Wrap(x-1, 1, MAP_WIDTH), Wrap(y - 1, 1, MAP_HEIGHT));
            if (possibleEntity.valid && possibleEntity.hasCollision &&
                possibleEntity.ID != ID) {
                return true;
            }
        }
    }
    return false;
}

Display::Display(SDL_Window* _w, SDL_Renderer* _r, Map* _map) : window(_w), renderer(_r), map(_map) {}

Display::~Display() {
    SDL_DestroyWindow(window);
}

void Display::Update(bool updateScreen) {
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            SDL_Rect point {i, j, 1, 1};
            MapObject object = map->grid(i, j);

            if (object.valid) {
                //A- Unlike SDL_FillRect from the window based rendering,
                //A- RenderFillRect doesn't have a color input.
                //A- Color is set beforehand by SetRenderDrawColor
                //A- It's also one field for each RGB, so use (map)object.color.(r/g/b) instead of RGBColor.ConvertToSDL()
                SDL_SetRenderDrawColor(renderer, object.color.r, object.color.g, object.color.b, 255);
                SDL_RenderFillRect(renderer, &point);
            }
            else {
                MapObject bg = map->background(i, j);
                SDL_SetRenderDrawColor(renderer, bg.color.r, bg.color.g, bg.color.b, 255);
                SDL_RenderFillRect(renderer, &point);
            }
        }
    }
    if (updateScreen) {
        //A- All UpdateWindowSurface calls were changed to RenderPresent
        SDL_RenderPresent(renderer);
    }
}

void Display::Erase(Player player, bool updateScreen) {

    for (int i = player.GetOldPos().x; i < player.GetOldPos().x + player.GetWidth(); i++) {
        for (int j = player.GetOldPos().y; j < player.GetOldPos().y + player.GetDepth(); j++) {
            MapObject background = map->background(i % MAP_WIDTH, j % MAP_HEIGHT);
            SDL_Rect rect = background.GetSDLRect();
            SDL_SetRenderDrawColor(renderer, background.color.r, background.color.g, background.color.b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    if (updateScreen) {
        //A-
        SDL_RenderPresent(renderer);
    }
}

void Display::Update(Player player, bool updateScreen) {
    if (player.hasMovedOffScreen) {
        Erase(player, false /* false so we don't update in Erase and update again here*/);
    }
    Update((MapEntity) player, updateScreen);
    player.hasMovedOffScreen = false;  // we just drew it, so it hasn't moved from what's on the screen for now
    if (updateScreen) {
        //A-
        SDL_RenderPresent(renderer);
    }
}

void Display::Update(Wall wall, bool updateScreen) {
    Update((MapObject) wall, updateScreen);
}

void Display::Update(MapObject object, bool updateScreen) {
    SDL_Rect rect = object.GetSDLRect();
    SDL_SetRenderDrawColor(renderer, object.color.r, object.color.g, object.color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
    if (updateScreen) {
        //A-
        SDL_RenderPresent(renderer);
    }
}

MapObject::MapObject(HitBox _hb, RGBColor _c, Map* _map, bool _hasCol) :
    hitbox(_hb), color(_c), map(_map), hasCollision(_hasCol) {
    ID = map->numberOfEntities++;
}

Wall::Wall(GridPos _pos, int _length, bool _isV, RGBColor _c, Map* _map) : 
    MapObject(HitBox(_pos, GridDimension(thickness, _length)), _c, _map),
    isVert(_isV) {
    // SetWidth(thickness); // fix the place holder
    if (!isVert) {
        std::swap(hitbox.dim.depth, hitbox.dim.width);
    }
}