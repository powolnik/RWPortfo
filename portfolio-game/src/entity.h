#pragma once
#include "types.h"

// ─── Entity Base Struct (Phase 3.3 stub — for future use) ────────────────────

struct Entity {
    float x = 0, y = 0;
    float velX = 0, velY = 0;
    int width = 16, height = 16;
    bool active = true;
    SpriteSheet* sheet = nullptr;
    AnimationController anim;
};

void renderEntity(SDL_Renderer* r, const Entity& e, int camX, int camY);
