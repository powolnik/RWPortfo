#pragma once
#include <SDL2/SDL.h>

// ─── Constants ───────────────────────────────────────────────────────────────

constexpr int   WINDOW_W      = 1280;
constexpr int   WINDOW_H      = 720;
constexpr float GRAVITY       = 980.0f;
constexpr float PLAYER_SPEED  = 220.0f;
constexpr float JUMP_FORCE    = 420.0f;
constexpr float SWIM_SPEED    = 160.0f;
constexpr float PICKUP_RANGE  = 48.0f;
// CHANGED: world now matches Beach tilemap dimensions (240×45 tiles × 16px)
constexpr int   WORLD_W       = 3840; // 240 tiles × 16px
constexpr int   WORLD_H       = 720;  // 45 tiles × 16px (1 viewport tall)

// Zone boundaries (world X coords)
constexpr int ZONE_BEACH_START   = 0;
constexpr int ZONE_BEACH_END     = 3840;   // 240 tiles × 16px (current tilemap)
constexpr int ZONE_FOREST_START  = 3840;   // placeholder — tilemap not yet loaded
constexpr int ZONE_FOREST_END    = 7680;
constexpr int ZONE_WETLAND_START = 7680;   // placeholder — tilemap not yet loaded
constexpr int ZONE_WETLAND_END   = 11520;

// Colors (solarpunk palette)
struct Color { Uint8 r, g, b, a; };

static const Color COL_SKY_TOP     = {135, 206, 235, 255};
static const Color COL_SKY_BOT     = {200, 230, 250, 255};
static const Color COL_WATER       = { 40, 120, 180, 180};
static const Color COL_WATER_DEEP  = { 20,  60, 120, 200};
static const Color COL_SAND        = {220, 200, 160, 255};
static const Color COL_GRASS       = { 60, 160,  60, 255};
static const Color COL_DARK_GRASS  = { 40, 120,  40, 255};
static const Color COL_MUD         = {100,  80,  50, 255};
static const Color COL_TREE_TRUNK  = {100,  70,  40, 255};
static const Color COL_TREE_LEAVES = { 30, 140,  50, 255};
static const Color COL_PLAYER_BODY = { 50, 180,  80, 255};
static const Color COL_PLAYER_HEAD = {230, 190, 150, 255};
static const Color COL_TRASH       = {180,  60,  60, 255};
static const Color COL_TRASH_GLOW  = {255, 100, 100, 120};
static const Color COL_ANIMAL      = {200, 160,  80, 255};
static const Color COL_BIRD        = { 80, 140, 200, 255};
static const Color COL_FISH        = {100, 200, 180, 255};
static const Color COL_UI_BG       = { 10,  10,  26, 200};
static const Color COL_UI_ACCENT   = { 96,  96, 255, 255};
static const Color COL_ECO_LOW     = {200,  60,  60, 255};
static const Color COL_ECO_MID     = {220, 180,  40, 255};
static const Color COL_ECO_HIGH    = { 60, 200,  80, 255};
static const Color COL_WHITE       = {255, 255, 255, 255};
static const Color COL_BLACK       = {  0,   0,   0, 255};

// ─── Enums ───────────────────────────────────────────────────────────────────

enum class GameScene { MENU, PLAYING, PAUSED, WIN };
enum class Zone { BEACH, FOREST, WETLAND };
enum class TrashType { BOTTLE, CAN, BAG, TIRE, BARREL };
enum class AnimalType { CRAB, BIRD, DEER, FROG, FISH };
