#include "World.hpp"
#include <algorithm>

unsigned RGBColor::ConvertToSDL(SDL_Surface* surface) {
    return SDL_MapRGB(surface->format, r, g, b);
}

MapObject::MapObject(HitBox _hb, RGBColor _c, Map* _m, bool _hasCol, bool _im) :
    hitbox(_hb), color(_c), map(_m), hasCollision(_hasCol), immobile(_im) {
    ID = map->numberOfEntities++;
}

bool MapObject::Valid() {
    if (this->ID && this->map) {
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

        //A- I added a check to see if the object that is moving has collision.
        //A- Ideally both objects need to have collision on for them to collide.
        if (this->hasCollision) {
            if (map->CheckForCollision(collisionPath, ID)) {
                //A- Collisions set velocity to 0. Makes bumping into walls less annoying
                this->X_velocity = 0.0;
                this->Y_velocity = 0.0;
                return;
            }
        }

        newPos.x = Wrap(oldPos.x, xD, MAP_WIDTH);
        newPos.y = Wrap(oldPos.y, yD, MAP_HEIGHT);
        //map->Delete(*this);
        SetPos(newPos);
        //map->Add(*this);
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
    numberOfEntities = 0;
    grid = Arr2d<MapObject>(MAP_WIDTH, MAP_HEIGHT);
    background = Arr2d<MapObject>(MAP_WIDTH, MAP_HEIGHT);
}

void Map::CreateBackground() {
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            HitBox hb = HitBox(x, y, 1, 1);
            int index = x*MAP_HEIGHT+y;
            RGBColor color = RGBColor(index % 255 / 2, (1 / (index+1)) % 255, (index / 10000) % 255);
            background(x,y) = MapObject(hb, color, this, false /* don't give background collision */);
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
    if (object.Valid()) {
        for (int i = object.GetCurrentPos().x; i < object.GetCurrentPos().x + object.GetWidth(); i++) {
            for (int j = object.GetCurrentPos().y; j < object.GetCurrentPos().y + object.GetDepth(); j++) {
                grid(i % MAP_WIDTH, j % MAP_HEIGHT) = object;
            }
        }
    }
}

bool Map::CheckForCollision(const HitBox& movingPiece, int ID)  {
    int xBound = movingPiece.origin.x + movingPiece.dim.width;
    int yBound = movingPiece.origin.y + movingPiece.dim.depth;
    for (int x = movingPiece.origin.x; x < xBound; x++) {
        for (int y = movingPiece.origin.y; y < yBound; y++) {
                        
            MapObject& possibleEntity = grid(Wrap(x-1, 1, MAP_WIDTH), Wrap(y - 1, 1, MAP_HEIGHT));
            if (possibleEntity.Valid() && possibleEntity.hasCollision &&
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

void Display::Update() {
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            SDL_Rect point {i, j, 1, 1};
            MapObject object = map->grid(i, j);

            if (object.Valid()) {
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
    Render();
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
        Erase(player, false);
    }
    Update((MapObject) player);
    player.hasMovedOffScreen = false;  // we just drew it, so it hasn't moved from what's on the screen for now
    Render();
}

void Display::Update(Wall wall) {
    Update((MapObject) wall);
    Render();
}

void Display::Update(MapObject object) {
    SDL_Rect rect = object.GetSDLRect();
    SDL_SetRenderDrawColor(renderer, object.color.r, object.color.g, object.color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
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

/*
Tile::Tile(HitBox _hb, SDL_Texture* _tex, int _col) {
	this->SetCollisionType(_col);
	this->SetTexture(_tex);
	this->hitbox = _hb;
}

Tile::~Tile() {
	SDL_DestroyTexture(texture);
}

TileMap::TileMap() {
	
}

//int TileMap::ReadMapFile() {

//}

void TileMap::GenerateTileMap(int arr[TILESWIDTH][TILESHEIGHT]) {

}

void TileMap::DisplayMap() {

}
*/
