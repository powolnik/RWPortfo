#pragma once
#include "world.h"
#include "player.h"
#include "hud.h"
#include "entity.h"

// ─── Game Module ─────────────────────────────────────────────────────────────

// GameState g is defined in game.cpp (extern declared in types.h)

void handleEvents();
void updateMenu(float dt);
void updatePlaying(float dt);
void updatePaused(float dt);
void updateWin(float dt);
void update(float dt);

void drawMenu();
void drawPaused();
void drawWin();
void render();

void mainLoopCallback();
