#pragma once
#include <string>
#include "dialogue.h"
#include "economy.h"

enum class NPCFavourState {
    NOT_MET,       // player hasn't talked to NPC yet
    INTRO_DONE,    // player heard intro, quest active
    FAVOUR_DONE,   // player completed favour condition
    REWARDED       // reward given, dialogue changed
};

struct NPC {
    std::string name;
    int x, y;              // world position (pixels)
    int zoneIndex;         // which zone NPC lives in (0=Beach, 2=Meadow, 3=Forest)
    NPCFavourState state = NPCFavourState::NOT_MET;

    // Favour condition counters
    int favourProgress = 0;
    int favourTarget   = 0;

    // Pixel-art placeholder: draw a coloured 16x16 NPC sprite
    SDL_Color colour;      // unique colour per NPC

    bool isNear(int px, int py, int radius = 40) const {
        int dx = px - x, dy = py - y;
        return dx*dx + dy*dy <= radius*radius;
    }

    void render(SDL_Renderer* renderer, int camX, int camY) {
        int sx = x - camX;
        int sy = y - camY;
        // Body
        SDL_Rect body = { sx, sy - 16, 16, 24 };
        SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, 255);
        SDL_RenderFillRect(renderer, &body);
        // Head
        SDL_Rect head = { sx + 2, sy - 28, 12, 12 };
        SDL_SetRenderDrawColor(renderer, 220, 180, 140, 255);
        SDL_RenderFillRect(renderer, &head);
        // Eyes (2 dots)
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderDrawPoint(renderer, sx + 5, sy - 24);
        SDL_RenderDrawPoint(renderer, sx + 10, sy - 24);
        // Exclamation mark if quest available
        if (state == NPCFavourState::NOT_MET || state == NPCFavourState::INTRO_DONE) {
            SDL_Rect mark = { sx + 6, sy - 38, 4, 8 };
            SDL_SetRenderDrawColor(renderer, 255, 220, 40, 255);
            SDL_RenderFillRect(renderer, &mark);
        }
        // Checkmark if rewarded
        if (state == NPCFavourState::REWARDED) {
            SDL_Rect check = { sx + 5, sy - 38, 8, 4 };
            SDL_SetRenderDrawColor(renderer, 80, 220, 100, 255);
            SDL_RenderFillRect(renderer, &check);
        }
    }
};

// Build the three NPCs with their default positions
inline NPC make_marta() {
    NPC n;
    n.name         = "Marta";
    n.x            = 320;   // Beach zone, near hut
    n.y            = 300;
    n.zoneIndex    = 0;
    n.favourTarget = 5;     // needs 5 plastic pickups
    n.colour       = {80, 140, 200, 255};  // blue-ish (fisherman)
    return n;
}

inline NPC make_tomek() {
    NPC n;
    n.name         = "Tomek";
    n.x            = 2800;  // Meadow zone
    n.y            = 280;
    n.zoneIndex    = 2;
    n.favourTarget = 3;     // needs bio-filter on 3 chemical drums
    n.colour       = {160, 100, 60, 255};  // earthy brown (camp)
    return n;
}

inline NPC make_sela() {
    NPC n;
    n.name         = "Baba Sela";
    n.x            = 2600;  // Forest zone (Birch grove) — within 0-3840
    n.y            = 260;
    n.zoneIndex    = 3;
    n.favourTarget = 1;     // just talk to her — she reveals hidden litter
    n.colour       = {160, 80, 160, 255};  // purple (elder / mystic)
    return n;
}
