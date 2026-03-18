#include "helpers.h"
#include <cmath>
#include <cstdlib>

// ─── Drawing Helpers ─────────────────────────────────────────────────────────

void setColor(Color c) {
    SDL_SetRenderDrawBlendMode(g.renderer, c.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(g.renderer, c.r, c.g, c.b, c.a);
}

void drawRect(int x, int y, int w, int h, Color c) {
    setColor(c);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(g.renderer, &r);
}

void drawRectWorld(float wx, float wy, int w, int h, Color c) {
    int sx = (int)(wx - g.camera.x);
    int sy = (int)(wy - g.camera.y);
    drawRect(sx, sy, w, h, c);
}

void drawCircle(int cx, int cy, int radius, Color c) {
    setColor(c);
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)std::sqrt(radius*radius - dy*dy);
        SDL_RenderDrawLine(g.renderer, cx-dx, cy+dy, cx+dx, cy+dy);
    }
}

void drawCircleWorld(float wx, float wy, int radius, Color c) {
    int sx = (int)(wx - g.camera.x);
    int sy = (int)(wy - g.camera.y);
    drawCircle(sx, sy, radius, c);
}

float lerp(float a, float b, float t) { return a + (b-a)*t; }

Color lerpColor(Color a, Color b, float t) {
    return {
        (Uint8)(a.r + (b.r - a.r)*t),
        (Uint8)(a.g + (b.g - a.g)*t),
        (Uint8)(a.b + (b.b - a.b)*t),
        (Uint8)(a.a + (b.a - a.a)*t)
    };
}

float randf() { return (float)rand() / (float)RAND_MAX; }
float randf(float lo, float hi) { return lo + randf()*(hi-lo); }

// ─── Terrain Query Helpers ───────────────────────────────────────────────────

int getGroundY(float worldX) {
    // Terrain profile: beach slopes down to water, forest is hilly, wetland is flat with ponds
    float x = worldX;

    if (x < 0) return 700;
    if (x >= WORLD_W) return 700;

    // Beach zone: gentle slope to water edge
    if (x < ZONE_BEACH_END) {
        float base = 620.0f;
        // Small dunes
        base += 15.0f * std::sin(x * 0.008f);
        base += 8.0f * std::sin(x * 0.02f);
        // Shore water area
        if (x < 300) {
            base = lerp(750.0f, base, x / 300.0f);
        }
        return (int)base;
    }

    // Forest zone: hilly
    if (x < ZONE_FOREST_END) {
        float lx = x - ZONE_FOREST_START;
        float base = 580.0f;
        base += 40.0f * std::sin(lx * 0.004f);
        base += 20.0f * std::sin(lx * 0.012f);
        base += 10.0f * std::sin(lx * 0.025f);
        return (int)base;
    }

    // Wetland zone: flat with water pools
    {
        float lx = x - ZONE_WETLAND_START;
        float base = 640.0f;
        base += 10.0f * std::sin(lx * 0.006f);
        base += 5.0f * std::sin(lx * 0.018f);
        return (int)base;
    }
}

bool isWaterAt(float worldX, float worldY) {
    // Beach: water on left edge
    if (worldX < 250 && worldY > 680) return true;

    // Wetland: pools
    if (worldX > ZONE_WETLAND_START) {
        float lx = worldX - ZONE_WETLAND_START;
        // Several pool locations
        float pool1Center = 400, pool2Center = 1000, pool3Center = 1600;
        float poolWidth = 200;
        int gy = getGroundY(worldX);
        if (worldY > gy - 5) {
            if (std::abs(lx - pool1Center) < poolWidth ||
                std::abs(lx - pool2Center) < poolWidth ||
                std::abs(lx - pool3Center) < poolWidth) {
                return true;
            }
        }
    }
    return false;
}

int getWaterLevel(float worldX) {
    if (worldX < 250) return 700;
    if (worldX > ZONE_WETLAND_START) {
        return getGroundY(worldX) + 20;
    }
    return 9999; // no water
}

Zone getZone(float worldX) {
    if (worldX < ZONE_BEACH_END) return Zone::BEACH;
    if (worldX < ZONE_FOREST_END) return Zone::FOREST;
    return Zone::WETLAND;
}

const char* getZoneName(Zone z) {
    switch(z) {
        case Zone::BEACH:   return "~ THE BEACH ~";
        case Zone::FOREST:  return "~ THE FOREST ~";
        case Zone::WETLAND: return "~ THE WETLANDS ~";
    }
    return "";
}
