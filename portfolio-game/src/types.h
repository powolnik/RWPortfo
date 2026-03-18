#pragma once
#include "constants.h"
#include "zone.h"
#include "spritesheet.h"
#include "animation.h"

#include <vector>
#include <string>
#include <cmath>

// ─── Structs ─────────────────────────────────────────────────────────────────

struct Vec2 {
    float x = 0, y = 0;
    Vec2 operator+(Vec2 o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(Vec2 o) const { return {x-o.x, y-o.y}; }
    Vec2 operator*(float s) const { return {x*s, y*s}; }
    float len() const { return std::sqrt(x*x + y*y); }
};

struct Player {
    Vec2 pos = {200, 500};
    Vec2 vel = {0, 0};
    bool onGround  = false;
    bool inWater   = false;
    bool facingRight = true;
    int  trashCollected = 0;
    int  totalTrash = 0;
    float animTime = 0;
    float pickupCooldown = 0;
    AnimationController anim;   // clip-based animation controller
    // Simple bounding box
    float w = 24, h = 48;
};

struct TrashItem {
    Vec2 pos;
    TrashType type;
    bool collected = false;
    float bobPhase = 0;   // for floating trash
    bool inWater = false;
};

struct Animal {
    Vec2 pos;
    AnimalType type;
    bool spawned = false;       // appears after enough cleanup
    float requiredEco = 0;      // eco% needed to appear
    float animPhase = 0;
    Vec2 moveDir = {0, 0};
    float moveTimer = 0;
};

struct Cloud {
    float x, y, w, speed;
};

struct Particle {
    Vec2 pos, vel;
    float life, maxLife;
    Color col;
};

struct TerrainSegment {
    int x1, x2;
    int groundY;       // Y coordinate of ground surface at this segment
    bool isWater;
};

struct Decoration {
    Vec2 pos;
    int type;  // 0=bush, 1=flower, 2=rock, 3=reed, 4=lily
    float scale;
};

// ─── Game State ──────────────────────────────────────────────────────────────

struct GameState {
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;

    // Timing
    Uint32  lastTick  = 0;
    float   deltaTime = 0.0f;
    float   totalTime = 0.0f;
    Uint64  frameCount = 0;

    // Input
    struct Input {
        bool left = false, right = false, up = false, down = false;
        bool jump = false;
        bool action = false;   // E key / pickup
        bool pause = false;
        bool enter = false;
        bool jumpPressed = false;  // single frame
        bool actionPressed = false;
        bool pausePressed = false;
        bool enterPressed = false;
    } input;

    // Scene
    GameScene scene = GameScene::MENU;

    // Camera
    Vec2 camera = {0, 0};

    // World
    Player player;
    std::vector<TrashItem>       trash;
    std::vector<Animal>          animals;
    std::vector<Cloud>           clouds;
    std::vector<Particle>        particles;
    std::vector<TerrainSegment>  terrain;
    std::vector<Decoration>      decorations;

    // CHANGED: TileMap-based terrain (replaces procedural heightmap)
    TileMap currentZoneMap;

    // Player sprite sheet (loaded from PNG or placeholder at runtime)
    SpriteSheet playerSheet;

    // Eco meter (0-100)
    float ecoMeter = 0;
    float targetEco = 0;

    // Messages
    std::string popupMsg;
    float popupTimer = 0;

    // Zone name display
    std::string currentZoneName;
    float zoneNameTimer = 0;
    Zone lastZone = Zone::BEACH;

    // Win state
    bool allTrashCollected = false;

    // Menu
    int menuSelection = 0;
    float menuAnim = 0;
};

extern GameState g;
