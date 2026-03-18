#pragma once
#include "helpers.h"
#include "player.h"
#include "zones/beach.h"

// ─── World Module ────────────────────────────────────────────────────────────

void spawnParticles(Vec2 pos, Color col, int count, float speed = 80.0f);
void generateWorld();
void drawSky();
void drawTerrain();
void drawTrees();
void drawDecorations();
void drawTrash();
void drawAnimals();
void drawParticles();
