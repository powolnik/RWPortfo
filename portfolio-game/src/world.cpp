#include "world.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// ─── Particle System ─────────────────────────────────────────────────────────

void spawnParticles(Vec2 pos, Color col, int count, float speed) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.pos = pos;
        float angle = randf(0, 6.28f);
        float spd = randf(speed * 0.3f, speed);
        p.vel = {std::cos(angle)*spd, std::sin(angle)*spd - 40.0f};
        p.maxLife = p.life = randf(0.3f, 0.8f);
        p.col = col;
        g.particles.push_back(p);
    }
}

// ─── World Generation ────────────────────────────────────────────────────────

void generateWorld() {
    srand((unsigned)time(nullptr));

    // CHANGED: build Beach tilemap (replaces procedural heightmap)
    g.currentZoneMap = buildBeachZone();

    // Init player sprite sheet — try real PNG first, fall back to placeholder
    if (g.playerSheet.texture) { destroySpriteSheet(g.playerSheet); }
    g.playerSheet = loadSpriteSheet(g.renderer, "assets/player.png", 16, 32);
    if (!g.playerSheet.texture) {
        SDL_Texture* ph = createPlaceholderPlayerSheet(g.renderer);
        g.playerSheet = { ph, 16, 32, 16, 16 };
    }

    // Register animation clips on player
    registerPlayerClips(g.player.anim);

    // Generate trash items across all zones
    g.trash.clear();

    // Beach trash (bottles, cans on sand + some in water)
    auto addTrash = [](float x, float y, TrashType t, bool water = false) {
        TrashItem item;
        item.pos = {x, y};
        item.type = t;
        item.inWater = water;
        item.bobPhase = randf(0, 6.28f);
        g.trash.push_back(item);
    };

    // Beach zone ~15 items
    for (int i = 0; i < 5; i++) {
        float x = randf(100, 500);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::BOTTLE, x < 250);
    }
    for (int i = 0; i < 5; i++) {
        float x = randf(400, 1200);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::CAN);
    }
    for (int i = 0; i < 5; i++) {
        float x = randf(800, 2000);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::BAG);
    }

    // Forest zone ~15 items
    for (int i = 0; i < 5; i++) {
        float x = randf(ZONE_FOREST_START + 100, ZONE_FOREST_START + 800);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::TIRE);
    }
    for (int i = 0; i < 5; i++) {
        float x = randf(ZONE_FOREST_START + 600, ZONE_FOREST_START + 1500);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::CAN);
    }
    for (int i = 0; i < 5; i++) {
        float x = randf(ZONE_FOREST_START + 1200, ZONE_FOREST_END - 100);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::BAG);
    }

    // Wetland zone ~15 items (some in water pools)
    for (int i = 0; i < 5; i++) {
        float x = randf(ZONE_WETLAND_START + 300, ZONE_WETLAND_START + 600);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::BARREL, true);
    }
    for (int i = 0; i < 5; i++) {
        float x = randf(ZONE_WETLAND_START + 800, ZONE_WETLAND_START + 1200);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::BOTTLE, true);
    }
    for (int i = 0; i < 5; i++) {
        float x = randf(ZONE_WETLAND_START + 1400, ZONE_WETLAND_END - 200);
        addTrash(x, (float)getGroundY(x) - 8, TrashType::CAN);
    }

    g.player.totalTrash = (int)g.trash.size();

    // Animals (appear as eco improves)
    g.animals.clear();
    auto addAnimal = [](float x, float y, AnimalType t, float ecoReq) {
        Animal a;
        a.pos = {x, y};
        a.type = t;
        a.requiredEco = ecoReq;
        a.animPhase = randf(0, 6.28f);
        a.moveTimer = randf(1, 4);
        g.animals.push_back(a);
    };

    // Beach animals
    addAnimal(350, 680, AnimalType::CRAB, 10);
    addAnimal(600, 680, AnimalType::CRAB, 20);
    addAnimal(180, 710, AnimalType::FISH, 15);
    addAnimal(800, 500, AnimalType::BIRD, 25);

    // Forest animals
    addAnimal(ZONE_FOREST_START + 400, 520, AnimalType::DEER, 35);
    addAnimal(ZONE_FOREST_START + 900, 500, AnimalType::BIRD, 30);
    addAnimal(ZONE_FOREST_START + 1400, 540, AnimalType::DEER, 50);
    addAnimal(ZONE_FOREST_START + 1800, 480, AnimalType::BIRD, 45);

    // Wetland animals
    addAnimal(ZONE_WETLAND_START + 400, 650, AnimalType::FROG, 55);
    addAnimal(ZONE_WETLAND_START + 1000, 660, AnimalType::FROG, 65);
    addAnimal(ZONE_WETLAND_START + 600, 640, AnimalType::FISH, 60);
    addAnimal(ZONE_WETLAND_START + 1500, 600, AnimalType::BIRD, 75);

    // Clouds
    g.clouds.clear();
    for (int i = 0; i < 12; i++) {
        Cloud c;
        c.x = randf(0, (float)WORLD_W);
        c.y = randf(30, 200);
        c.w = randf(80, 200);
        c.speed = randf(8, 25);
        g.clouds.push_back(c);
    }

    // Decorations
    g.decorations.clear();
    // Beach bushes and rocks
    for (int i = 0; i < 20; i++) {
        Decoration d;
        d.pos.x = randf(300, (float)ZONE_BEACH_END - 50);
        d.pos.y = (float)getGroundY(d.pos.x);
        d.type = (i % 3 == 0) ? 2 : 1; // rocks and flowers
        d.scale = randf(0.6f, 1.2f);
        g.decorations.push_back(d);
    }
    // Forest bushes and flowers
    for (int i = 0; i < 30; i++) {
        Decoration d;
        d.pos.x = randf((float)ZONE_FOREST_START + 50, (float)ZONE_FOREST_END - 50);
        d.pos.y = (float)getGroundY(d.pos.x);
        d.type = (i % 2 == 0) ? 0 : 1;
        d.scale = randf(0.8f, 1.4f);
        g.decorations.push_back(d);
    }
    // Wetland reeds and lilies
    for (int i = 0; i < 25; i++) {
        Decoration d;
        d.pos.x = randf((float)ZONE_WETLAND_START + 50, (float)ZONE_WETLAND_END - 50);
        d.pos.y = (float)getGroundY(d.pos.x);
        d.type = (i % 2 == 0) ? 3 : 4; // reeds and lilies
        d.scale = randf(0.7f, 1.3f);
        g.decorations.push_back(d);
    }
}

// ─── Render ──────────────────────────────────────────────────────────────────

void drawSky() {
    // Gradient sky
    for (int y = 0; y < WINDOW_H; y++) {
        float t = (float)y / (float)WINDOW_H;
        Color c = lerpColor(COL_SKY_TOP, COL_SKY_BOT, t);
        // Slight green tint as eco improves
        float eco01 = g.ecoMeter / 100.0f;
        c.g = (Uint8)std::min(255.0f, c.g + eco01 * 20.0f);
        setColor(c);
        SDL_RenderDrawLine(g.renderer, 0, y, WINDOW_W, y);
    }

    // Sun
    float sunX = WINDOW_W * 0.8f;
    float sunY = 80 + 20 * std::sin(g.totalTime * 0.1f);
    drawCircle((int)sunX, (int)sunY, 40, {255, 220, 80, 255});
    drawCircle((int)sunX, (int)sunY, 50, {255, 220, 80, 60});

    // Clouds (parallax)
    for (auto& c : g.clouds) {
        float parallax = 0.3f;
        float sx = c.x - g.camera.x * parallax;
        float sy = c.y;
        // Wrap
        while (sx > WINDOW_W + 200) sx -= WORLD_W + 400;
        while (sx < -300) sx += WORLD_W + 400;

        Color cc = {255, 255, 255, 80};
        drawCircle((int)sx, (int)sy, (int)(c.w*0.3f), cc);
        drawCircle((int)(sx + c.w*0.25f), (int)(sy - 5), (int)(c.w*0.35f), cc);
        drawCircle((int)(sx + c.w*0.5f), (int)sy, (int)(c.w*0.28f), cc);
    }
}

void drawTerrain() {
    // Draw ground fill
    for (int sx = 0; sx < WINDOW_W; sx += 2) {
        float wx = sx + g.camera.x;
        int gy = getGroundY(wx);
        int sy = gy - (int)g.camera.y;

        Zone z = getZone(wx);
        Color topCol, botCol;
        switch (z) {
            case Zone::BEACH:
                topCol = COL_SAND;
                botCol = {190, 170, 130, 255};
                break;
            case Zone::FOREST:
                topCol = COL_GRASS;
                botCol = COL_DARK_GRASS;
                break;
            case Zone::WETLAND:
                topCol = COL_MUD;
                botCol = {70, 55, 35, 255};
                break;
        }

        // Ground fill
        for (int y = sy; y < WINDOW_H; y += 2) {
            float t = (float)(y - sy) / 200.0f;
            if (t > 1) t = 1;
            Color c = lerpColor(topCol, botCol, t);
            setColor(c);
            SDL_Rect r = {sx, y, 2, 2};
            SDL_RenderFillRect(g.renderer, &r);
        }

        // Grass tufts on forest
        if (z == Zone::FOREST && sx % 8 == 0) {
            int grassH = 6 + (int)(4 * std::sin(wx * 0.1f + g.totalTime));
            setColor({30, 160, 40, 200});
            SDL_RenderDrawLine(g.renderer, sx, sy, sx, sy - grassH);
            SDL_RenderDrawLine(g.renderer, sx+1, sy, sx+3, sy - grassH + 2);
        }
    }

    // Draw water
    for (int sx = 0; sx < WINDOW_W; sx += 2) {
        float wx = sx + g.camera.x;
        if (wx < 300 || wx > ZONE_WETLAND_START) {
            int wl = getWaterLevel(wx) - (int)g.camera.y;
            if (wl < WINDOW_H) {
                // Water surface wave
                float wave = 3 * std::sin(wx * 0.03f + g.totalTime * 2.0f);
                int waterTop = wl + (int)wave;
                // Semi-transparent water
                for (int y = waterTop; y < WINDOW_H; y += 2) {
                    float depth = (float)(y - waterTop) / 100.0f;
                    if (depth > 1) depth = 1;
                    Color c = lerpColor(COL_WATER, COL_WATER_DEEP, depth);
                    // Ripple effect
                    c.b = (Uint8)std::min(255.0f, c.b + 15 * std::sin(wx*0.05f + y*0.1f + g.totalTime*3));
                    setColor(c);
                    SDL_Rect r = {sx, y, 2, 2};
                    SDL_RenderFillRect(g.renderer, &r);
                }
                // Surface highlight
                setColor({180, 220, 255, 100});
                SDL_RenderDrawLine(g.renderer, sx, waterTop, sx+2, waterTop);
            }
        }
    }
}

void drawTrees() {
    // Forest trees
    if (g.camera.x + WINDOW_W > ZONE_FOREST_START && g.camera.x < ZONE_FOREST_END) {
        for (float tx = ZONE_FOREST_START + 80; tx < ZONE_FOREST_END; tx += 120) {
            float sx = tx - g.camera.x;
            if (sx < -80 || sx > WINDOW_W + 80) continue;

            int gy = getGroundY(tx);
            float sy = gy - g.camera.y;

            // Trunk
            drawRect((int)sx - 6, (int)sy - 100, 12, 100, COL_TREE_TRUNK);

            // Canopy (layered circles)
            float sway = 3 * std::sin(g.totalTime + tx * 0.01f);
            Color leafCol = COL_TREE_LEAVES;
            // Greener with eco
            leafCol.g = (Uint8)std::min(255.0f, leafCol.g + g.ecoMeter * 0.5f);

            drawCircle((int)(sx + sway), (int)(sy - 120), 35, leafCol);
            drawCircle((int)(sx - 15 + sway*0.8f), (int)(sy - 105), 28, leafCol);
            drawCircle((int)(sx + 18 + sway*1.2f), (int)(sy - 108), 30, leafCol);
            drawCircle((int)(sx + sway*0.5f), (int)(sy - 140), 25, {20, 120, 40, 255});
        }
    }

    // Wetland reeds
    if (g.camera.x + WINDOW_W > ZONE_WETLAND_START) {
        for (float tx = ZONE_WETLAND_START + 30; tx < ZONE_WETLAND_END; tx += 40) {
            float sx = tx - g.camera.x;
            if (sx < -20 || sx > WINDOW_W + 20) continue;

            int gy = getGroundY(tx);
            float sy = gy - g.camera.y;
            float sway = 4 * std::sin(g.totalTime * 1.5f + tx * 0.02f);

            setColor({60, 120, 50, 200});
            SDL_RenderDrawLine(g.renderer, (int)sx, (int)sy, (int)(sx + sway), (int)(sy - 40));
            SDL_RenderDrawLine(g.renderer, (int)sx+2, (int)sy, (int)(sx + sway + 2), (int)(sy - 35));
        }
    }

    // Beach palm trees
    if (g.camera.x < ZONE_BEACH_END) {
        float palmPositions[] = {500, 900, 1300, 1700};
        for (float tx : palmPositions) {
            float sx = tx - g.camera.x;
            if (sx < -80 || sx > WINDOW_W + 80) continue;

            int gy = getGroundY(tx);
            float sy = gy - g.camera.y;
            float sway = 5 * std::sin(g.totalTime * 0.8f + tx * 0.005f);

            // Curved trunk
            for (int i = 0; i < 80; i++) {
                float t = i / 80.0f;
                float cx = sx + sway * t * t;
                float cy = sy - i;
                drawRect((int)cx - 4, (int)cy, 8, 2, {140, 100, 60, 255});
            }

            // Palm fronds
            float topX = sx + sway;
            float topY = sy - 80;
            for (int f = 0; f < 5; f++) {
                float angle = -2.5f + f * 1.2f;
                float fSway = sway * 0.3f;
                for (int j = 0; j < 40; j++) {
                    float ft = j / 40.0f;
                    float fx = topX + std::cos(angle + fSway*0.02f) * j * 1.5f;
                    float fy = topY + std::sin(angle) * j * 0.8f + ft*ft * 20;
                    setColor({40, 160, 50, (Uint8)(255 - ft*100)});
                    SDL_RenderDrawLine(g.renderer, (int)fx, (int)fy, (int)fx+2, (int)fy+1);
                }
            }
        }
    }
}

void drawDecorations() {
    for (auto& d : g.decorations) {
        float sx = d.pos.x - g.camera.x;
        float sy = d.pos.y - g.camera.y;
        if (sx < -30 || sx > WINDOW_W + 30) continue;

        float s = d.scale;
        switch (d.type) {
            case 0: // bush
                drawCircle((int)sx, (int)(sy - 8*s), (int)(12*s), {50, 140, 50, 200});
                drawCircle((int)(sx + 8*s), (int)(sy - 6*s), (int)(10*s), {40, 130, 40, 200});
                break;
            case 1: // flower
            {
                setColor({30, 150, 30, 255});
                SDL_RenderDrawLine(g.renderer, (int)sx, (int)sy, (int)sx, (int)(sy - 15*s));
                Color flowerCols[] = {{255,100,100,255},{255,200,50,255},{200,100,255,255},{255,150,200,255}};
                drawCircle((int)sx, (int)(sy - 15*s), (int)(4*s), flowerCols[(int)(d.pos.x)%4]);
                break;
            }
            case 2: // rock
                drawCircle((int)sx, (int)(sy - 5*s), (int)(8*s), {130, 130, 120, 255});
                drawCircle((int)(sx + 3), (int)(sy - 7*s), (int)(5*s), {150, 150, 140, 255});
                break;
            case 3: // reed
            {
                float sway = 3 * std::sin(g.totalTime * 1.8f + d.pos.x * 0.03f);
                setColor({70, 130, 60, 220});
                SDL_RenderDrawLine(g.renderer, (int)sx, (int)sy, (int)(sx+sway), (int)(sy-30*s));
                break;
            }
            case 4: // lily pad
                drawCircle((int)sx, (int)sy, (int)(6*s), {60, 150, 60, 180});
                break;
        }
    }
}

void drawTrash() {
    for (auto& t : g.trash) {
        if (t.collected) continue;

        float sx = t.pos.x - g.camera.x;
        float sy = t.pos.y - g.camera.y;
        if (sx < -30 || sx > WINDOW_W + 30) continue;

        // Bob in water
        if (t.inWater) {
            sy += 4 * std::sin(g.totalTime * 2 + t.bobPhase);
        }

        // Glow indicator
        float pulse = 0.5f + 0.5f * std::sin(g.totalTime * 3 + t.pos.x * 0.01f);
        drawCircle((int)sx, (int)sy, (int)(14 + pulse*4), {255, 100, 100, (Uint8)(40 + pulse*40)});

        // Draw trash based on type
        switch (t.type) {
            case TrashType::BOTTLE:
                drawRect((int)sx-3, (int)sy-10, 6, 14, COL_TRASH);
                drawRect((int)sx-2, (int)sy-13, 4, 4, {200, 80, 80, 255});
                break;
            case TrashType::CAN:
                drawRect((int)sx-4, (int)sy-8, 8, 10, {180, 180, 180, 255});
                drawRect((int)sx-4, (int)sy-8, 8, 3, COL_TRASH);
                break;
            case TrashType::BAG:
                drawCircle((int)sx, (int)sy-5, 8, {200, 200, 200, 200});
                drawCircle((int)sx+3, (int)(sy-7), 5, {180, 180, 180, 200});
                break;
            case TrashType::TIRE:
                drawCircle((int)sx, (int)sy-8, 12, {50, 50, 50, 255});
                drawCircle((int)sx, (int)sy-8, 7, {80, 70, 60, 255});
                break;
            case TrashType::BARREL:
                drawRect((int)sx-8, (int)sy-16, 16, 20, {120, 80, 40, 255});
                drawRect((int)sx-9, (int)sy-14, 18, 3, {100, 70, 30, 255});
                drawRect((int)sx-9, (int)sy-6, 18, 3, {100, 70, 30, 255});
                break;
        }

        // "E" prompt when player is near
        float dist = (t.pos - g.player.pos).len();
        if (dist < PICKUP_RANGE * 1.5f) {
            float alpha = std::max(0.0f, 1.0f - dist / (PICKUP_RANGE * 1.5f));
            drawRect((int)sx-8, (int)sy-28, 16, 14, {10, 10, 26, (Uint8)(200*alpha)});
            // E letter (crude pixel art)
            Color ec = {96, 96, 255, (Uint8)(255*alpha)};
            drawRect((int)sx-4, (int)sy-26, 8, 2, ec);
            drawRect((int)sx-4, (int)sy-22, 6, 2, ec);
            drawRect((int)sx-4, (int)sy-18, 8, 2, ec);
            drawRect((int)sx-4, (int)sy-26, 2, 10, ec);
        }
    }
}

void drawAnimals() {
    for (auto& a : g.animals) {
        if (!a.spawned) continue;

        float sx = a.pos.x - g.camera.x;
        float sy = a.pos.y - g.camera.y;
        if (sx < -30 || sx > WINDOW_W + 30) continue;

        switch (a.type) {
            case AnimalType::CRAB:
            {
                // Orange crab body
                drawCircle((int)sx, (int)sy, 6, {220, 120, 40, 255});
                // Legs
                float legMove = std::sin(a.animPhase * 3) * 3;
                setColor({200, 100, 30, 255});
                SDL_RenderDrawLine(g.renderer, (int)sx-6, (int)sy, (int)(sx-10+legMove), (int)(sy+4));
                SDL_RenderDrawLine(g.renderer, (int)sx+6, (int)sy, (int)(sx+10-legMove), (int)(sy+4));
                // Claws
                drawCircle((int)(sx-10+legMove), (int)(sy-2), 3, {230, 130, 40, 255});
                drawCircle((int)(sx+10-legMove), (int)(sy-2), 3, {230, 130, 40, 255});
                // Eyes
                drawCircle((int)sx-2, (int)(sy-5), 2, COL_WHITE);
                drawCircle((int)sx+2, (int)(sy-5), 2, COL_WHITE);
                break;
            }
            case AnimalType::BIRD:
            {
                float flap = std::sin(a.animPhase * 4) * 8;
                // Body
                drawCircle((int)sx, (int)sy, 5, COL_BIRD);
                // Wings
                setColor(COL_BIRD);
                SDL_RenderDrawLine(g.renderer, (int)sx, (int)sy, (int)(sx-12), (int)(sy - 5 + flap));
                SDL_RenderDrawLine(g.renderer, (int)sx, (int)sy, (int)(sx+12), (int)(sy - 5 - flap));
                // Beak
                setColor({240, 180, 40, 255});
                SDL_RenderDrawLine(g.renderer, (int)sx+5, (int)sy, (int)sx+9, (int)(sy+1));
                break;
            }
            case AnimalType::DEER:
            {
                // Body
                drawCircle((int)sx, (int)sy-10, 10, COL_ANIMAL);
                drawCircle((int)sx-8, (int)sy-12, 7, COL_ANIMAL);
                // Head
                drawCircle((int)sx-14, (int)sy-18, 6, {210, 170, 90, 255});
                // Legs
                float legAnim = std::sin(a.animPhase * 2) * 3;
                setColor({170, 130, 70, 255});
                SDL_RenderDrawLine(g.renderer, (int)sx-5, (int)sy, (int)(sx-7), (int)(sy+15+legAnim));
                SDL_RenderDrawLine(g.renderer, (int)sx+5, (int)sy, (int)(sx+3), (int)(sy+15-legAnim));
                // Antlers
                setColor({160, 120, 60, 255});
                SDL_RenderDrawLine(g.renderer, (int)sx-14, (int)sy-24, (int)(sx-18), (int)(sy-32));
                SDL_RenderDrawLine(g.renderer, (int)sx-14, (int)sy-24, (int)(sx-10), (int)(sy-32));
                // Eye
                drawCircle((int)sx-16, (int)sy-18, 1, COL_BLACK);
                break;
            }
            case AnimalType::FROG:
            {
                // Body
                float hop = std::abs(std::sin(a.animPhase * 2)) * 5;
                drawCircle((int)sx, (int)(sy - hop), 7, {50, 180, 50, 255});
                // Eyes
                drawCircle((int)sx-3, (int)(sy - 6 - hop), 3, {60, 200, 60, 255});
                drawCircle((int)sx+3, (int)(sy - 6 - hop), 3, {60, 200, 60, 255});
                drawCircle((int)sx-3, (int)(sy - 6 - hop), 1, COL_BLACK);
                drawCircle((int)sx+3, (int)(sy - 6 - hop), 1, COL_BLACK);
                break;
            }
            case AnimalType::FISH:
            {
                float swim = std::sin(a.animPhase * 3) * 3;
                // Body
                setColor(COL_FISH);
                for (int i = -6; i <= 6; i++) {
                    int h = (int)(4 * std::sqrt(1.0f - (i*i)/36.0f));
                    SDL_RenderDrawLine(g.renderer, (int)(sx+i+swim), (int)(sy-h), (int)(sx+i+swim), (int)(sy+h));
                }
                // Tail
                setColor({80, 180, 160, 255});
                SDL_RenderDrawLine(g.renderer, (int)(sx+6+swim), (int)sy, (int)(sx+12+swim), (int)(sy-4));
                SDL_RenderDrawLine(g.renderer, (int)(sx+6+swim), (int)sy, (int)(sx+12+swim), (int)(sy+4));
                // Eye
                drawCircle((int)(sx-3+swim), (int)(sy-1), 1, COL_BLACK);
                break;
            }
        }
    }
}

void drawParticles() {
    for (auto& p : g.particles) {
        float sx = p.pos.x - g.camera.x;
        float sy = p.pos.y - g.camera.y;
        float alpha = p.life / p.maxLife;
        Color c = p.col;
        c.a = (Uint8)(c.a * alpha);
        int size = (int)(3 * alpha) + 1;
        drawRect((int)sx - size/2, (int)sy - size/2, size, size, c);
    }
}
