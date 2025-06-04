#include "World.hpp"
#include <algorithm>

unsigned RGBColor::ConvertToSDL(SDL_Surface* surface) {
    return SDL_MapRGB(surface->format, r, g, b);
}

//A- Just checks if a map object has a map.
bool MapObject::Valid() {
    if (this->map) {
        return true;
    }
    return false;
}

void MapObject::Move(int xD, int yD) {
    if (!immobile) {
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
    else {
        //A "this" just prints the memory address. I know there's a better way but too lazy to look that up.
        std::cout << "Attempt to move immobile MapObject; " << this << std::endl;
    }
}

void MapObject::ForceMove(int xD, int yD) {
    //A- If the object *does* have collision, this just turns it off temporarily to move it, then turns it back on.
    if (this->hasCollision == true) {
        this->hasCollision = false;
        Move(xD, yD);
        this->hasCollision = true;
    }
    else Move(xD, yD);
}

Map::Map () {
    //A- Map_width & height are the same as the window width & height currently
    grid = Arr2d<MapObject*>(MAP_WIDTH, MAP_HEIGHT);
}

void Map::AddToGrid(MapObject& object) {
    for (int i = object.GetCurrentPos().x; i < object.GetCurrentPos().x + object.GetWidth(); i++) {
        for (int j = object.GetCurrentPos().y; j < object.GetCurrentPos().y + object.GetDepth(); j++) {
            grid(i % MAP_WIDTH, j % MAP_HEIGHT) = &object;
        }
    }
}

void Map::AddObject(MapObject* object) {
    object->map = this;
    //A- Conveniently the size of the vector before an object is added will equal its index.
    object->ID = allObjects.size();
    allObjects.push_back(object);
    AddToGrid(*object);
}

//A- In the future we may want special behavior for adding players.
void Map::AddObject(Player* player) {
    AddObject((MapObject*)player);
}
void Map::AddObject(Wall* wall) {
    AddObject((MapObject*)wall);
}

bool Map::CheckForCollision(const HitBox& movingPiece, int ID) {
    int xBound = movingPiece.origin.x + movingPiece.dim.width;
    int yBound = movingPiece.origin.y + movingPiece.dim.depth;
        for (int x = movingPiece.origin.x; x < xBound; x++) {
            for (int y = movingPiece.origin.y; y < yBound; y++) {
                MapObject* possibleEntity = grid(Wrap(x - 1, 1, MAP_WIDTH), Wrap(y - 1, 1, MAP_HEIGHT));
                if (possibleEntity->Valid() && possibleEntity->hasCollision && possibleEntity->ID != ID) {
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

void Display::Update() {
    //This will always draw & render all objects 1 by 1, even if they haven't changed in any way.
    //Isn't the most optimal thing but an easy enough fix later
    //Would just need some kind of "updated" flag in MapObject (or Map).

    for (int i = 0; i < map->GetNumberOfObjects(); i++) {
        MapObject object = map->GetObject(i);
        if (object.Valid()) {
            SDL_SetRenderDrawColor(renderer, object.color.r, object.color.g, object.color.b, 255);
            SDL_Rect point{object.GetSDLRect()};
            SDL_RenderFillRect(renderer, &point);
        }
    }
    Render();
}

void Display::Render() {
    SDL_RenderPresent(renderer);
}

Wall::Wall(GridPos _pos, int _length, bool _isV, RGBColor _c) : 
    MapObject(HitBox(_pos, GridDimension(thickness, _length)), _c),
    isVert(_isV) {
    // SetWidth(thickness); // fix the place holder
    if (!isVert) {
        std::swap(hitbox.dim.depth, hitbox.dim.width);
    }
}

/* UNUSED FUNCTIONS */
/*
void Map::CreateBackground() {
    background = Arr2d<MapObject>(MAP_WIDTH, MAP_HEIGHT);
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            HitBox hb = HitBox(x, y, 1, 1);
            int index = x * MAP_HEIGHT + y;
            RGBColor color = RGBColor(index % 255 / 2, (1 / (index + 1)) % 255, (index / 10000) % 255);
            background(x, y) = MapObject(hb, color, this, false);
        }
    }
}

void Display::Erase(Player player, bool renderChange) {
    for (int i = player.GetOldPos().x; i < player.GetOldPos().x + player.GetWidth(); i++) {
        for (int j = player.GetOldPos().y; j < player.GetOldPos().y + player.GetDepth(); j++) {
            MapObject background = map->background(i % MAP_WIDTH, j % MAP_HEIGHT);
            SDL_Rect rect = background.GetSDLRect();
            SDL_SetRenderDrawColor(renderer, background.color.r, background.color.g, background.color.b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    if (renderChange) {
        Render();
    }
}

void Display::Update(Player player) {
    if (player.hasMovedOffScreen) {
        //Erase(player, false);
    }
    Update((MapObject) player);
    player.hasMovedOffScreen = false;  // we just drew it, so it hasn't moved from what's on the screen for now
}

void Display::Update(Wall wall) {
    Update((MapObject) wall);
}

void Display::Update(MapObject object) {
    SDL_Rect rect = object.GetSDLRect();
    SDL_SetRenderDrawColor(renderer, object.color.r, object.color.g, object.color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
    Render();
}



*/