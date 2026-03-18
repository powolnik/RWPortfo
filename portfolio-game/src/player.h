#pragma once
#include "helpers.h"

// ─── Player Module ───────────────────────────────────────────────────────────

SDL_Texture* createPlaceholderPlayerSheet(SDL_Renderer* renderer);
void updatePlayer(float dt);
void drawPlayer();
