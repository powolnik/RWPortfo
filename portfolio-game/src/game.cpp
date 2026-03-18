#include "game.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// ─── Global Game State Definition ────────────────────────────────────────────

GameState g;

// ─── Input ───────────────────────────────────────────────────────────────────

void handleEvents() {
    // Reset single-frame inputs
    g.input.jumpPressed = false;
    g.input.actionPressed = false;
    g.input.pausePressed = false;
    g.input.enterPressed = false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
#ifdef __EMSCRIPTEN__
                emscripten_cancel_main_loop();
#endif
                break;
            case SDL_KEYDOWN:
                if (e.key.repeat) break;
                switch (e.key.keysym.scancode) {
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:  g.input.left = true; break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT: g.input.right = true; break;
                    case SDL_SCANCODE_W:
                    case SDL_SCANCODE_UP:    g.input.up = true; break;
                    case SDL_SCANCODE_S:
                    case SDL_SCANCODE_DOWN:  g.input.down = true; break;
                    case SDL_SCANCODE_SPACE:
                        g.input.jump = true;
                        g.input.jumpPressed = true;
                        break;
                    case SDL_SCANCODE_E:
                        g.input.action = true;
                        g.input.actionPressed = true;
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        g.input.pause = true;
                        g.input.pausePressed = true;
                        break;
                    case SDL_SCANCODE_RETURN:
                        g.input.enter = true;
                        g.input.enterPressed = true;
                        break;
                    default: break;
                }
                break;
            case SDL_KEYUP:
                switch (e.key.keysym.scancode) {
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:  g.input.left = false; break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT: g.input.right = false; break;
                    case SDL_SCANCODE_W:
                    case SDL_SCANCODE_UP:    g.input.up = false; break;
                    case SDL_SCANCODE_S:
                    case SDL_SCANCODE_DOWN:  g.input.down = false; break;
                    case SDL_SCANCODE_SPACE: g.input.jump = false; break;
                    case SDL_SCANCODE_E:     g.input.action = false; break;
                    case SDL_SCANCODE_ESCAPE:g.input.pause = false; break;
                    case SDL_SCANCODE_RETURN:g.input.enter = false; break;
                    default: break;
                }
                break;
        }
    }
}

// ─── Update ──────────────────────────────────────────────────────────────────

void updateMenu(float dt) {
    g.menuAnim += dt;
    if (g.input.enterPressed || g.input.jumpPressed) {
        g.scene = GameScene::PLAYING;
        generateWorld();
        g.player.pos = {400, 500};
        g.ecoMeter = 0;
        g.targetEco = 0;
        g.currentZoneName = getZoneName(Zone::BEACH);
        g.zoneNameTimer = 3.0f;
    }
}

void updatePlaying(float dt) {
    // Player physics / animation / movement
    updatePlayer(dt);

    Player& p = g.player;

    // Pickup trash (E key)
    if (g.input.actionPressed && p.pickupCooldown <= 0) {
        float nearestDist = 99999;
        int nearestIdx = -1;
        for (int i = 0; i < (int)g.trash.size(); i++) {
            if (g.trash[i].collected) continue;
            float dist = (g.trash[i].pos - p.pos).len();
            if (dist < PICKUP_RANGE && dist < nearestDist) {
                nearestDist = dist;
                nearestIdx = i;
            }
        }
        if (nearestIdx >= 0) {
            g.trash[nearestIdx].collected = true;
            p.trashCollected++;
            p.pickupCooldown = 0.2f;

            // Particles!
            spawnParticles(g.trash[nearestIdx].pos, {60, 200, 80, 255}, 12, 100.0f);

            // Update eco meter
            g.targetEco = (float)p.trashCollected / (float)p.totalTrash * 100.0f;

            // Show popup
            const char* names[] = {"Bottle", "Can", "Plastic bag", "Tire", "Barrel"};
            char buf[64];
            snprintf(buf, sizeof(buf), "+1 %s collected! [%d/%d]",
                     names[(int)g.trash[nearestIdx].type],
                     p.trashCollected, p.totalTrash);
            g.popupMsg = buf;
            g.popupTimer = 2.0f;

            // Check win
            if (p.trashCollected >= p.totalTrash) {
                g.allTrashCollected = true;
                g.scene = GameScene::WIN;
                g.popupMsg = "Island fully restored!";
                g.popupTimer = 5.0f;
            }
        }
    }

    // Eco meter smooth animation
    if (g.ecoMeter < g.targetEco) {
        g.ecoMeter += dt * 30.0f;
        if (g.ecoMeter > g.targetEco) g.ecoMeter = g.targetEco;
    }

    // Update animals (spawn if eco is high enough, wander around)
    for (auto& a : g.animals) {
        a.animPhase += dt * 2.0f;
        if (!a.spawned && g.ecoMeter >= a.requiredEco) {
            a.spawned = true;
            spawnParticles(a.pos, {100, 255, 100, 255}, 20, 60.0f);
        }
        if (a.spawned) {
            a.moveTimer -= dt;
            if (a.moveTimer <= 0) {
                a.moveTimer = randf(2, 5);
                if (a.type == AnimalType::BIRD) {
                    a.moveDir = {randf(-40, 40), randf(-20, 10)};
                } else if (a.type == AnimalType::FISH) {
                    a.moveDir = {randf(-30, 30), randf(-10, 10)};
                } else {
                    a.moveDir = {randf(-20, 20), 0};
                }
            }
            a.pos.x += a.moveDir.x * dt;
            a.pos.y += a.moveDir.y * dt;
        }
    }

    // Update particles
    for (auto& p : g.particles) {
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.vel.y += 200.0f * dt;
        p.life -= dt;
    }
    g.particles.erase(
        std::remove_if(g.particles.begin(), g.particles.end(),
                        [](const Particle& p){ return p.life <= 0; }),
        g.particles.end()
    );

    // Update clouds
    for (auto& c : g.clouds) {
        c.x += c.speed * dt;
        if (c.x > WORLD_W + 200) c.x = -c.w - 50;
    }

    // Zone detection
    Zone curZone = getZone(p.pos.x);
    if (curZone != g.lastZone) {
        g.lastZone = curZone;
        g.currentZoneName = getZoneName(curZone);
        g.zoneNameTimer = 3.0f;
    }
    if (g.zoneNameTimer > 0) g.zoneNameTimer -= dt;

    // Popup timer
    if (g.popupTimer > 0) g.popupTimer -= dt;

    // Camera follow
    float targetCamX = p.pos.x - WINDOW_W / 2.0f;
    float targetCamY = p.pos.y - WINDOW_H * 0.6f;
    g.camera.x = lerp(g.camera.x, targetCamX, 4.0f * dt);
    g.camera.y = lerp(g.camera.y, targetCamY, 4.0f * dt);

    // Pause
    if (g.input.pausePressed) {
        g.scene = GameScene::PAUSED;
    }
}

void updatePaused(float dt) {
    (void)dt;
    if (g.input.pausePressed || g.input.enterPressed) {
        g.scene = GameScene::PLAYING;
    }
}

void updateWin(float dt) {
    g.menuAnim += dt;
    // Update particles still
    for (auto& p : g.particles) {
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.vel.y += 200.0f * dt;
        p.life -= dt;
    }
    g.particles.erase(
        std::remove_if(g.particles.begin(), g.particles.end(),
                        [](const Particle& p){ return p.life <= 0; }),
        g.particles.end()
    );
    // Celebratory particles
    if ((int)(g.menuAnim * 10) % 3 == 0) {
        Color cols[] = {{60,200,80,255},{80,160,255,255},{255,200,60,255}};
        spawnParticles({randf(g.camera.x, g.camera.x+WINDOW_W),
                        g.camera.y + WINDOW_H - 50},
                       cols[(int)(g.menuAnim*7)%3], 3, 150.0f);
    }

    if (g.input.enterPressed) {
        g.scene = GameScene::MENU;
    }
}

void update(float dt) {
    switch (g.scene) {
        case GameScene::MENU:    updateMenu(dt); break;
        case GameScene::PLAYING: updatePlaying(dt); break;
        case GameScene::PAUSED:  updatePaused(dt); break;
        case GameScene::WIN:     updateWin(dt); break;
    }
}

// ─── Render ──────────────────────────────────────────────────────────────────

void drawMenu() {
    // Background
    SDL_SetRenderDrawColor(g.renderer, 8, 8, 26, 255);
    SDL_RenderClear(g.renderer);

    // Animated background dots
    for (int y = 0; y < WINDOW_H; y += 48) {
        for (int x = 0; x < WINDOW_W; x += 48) {
            float pulse = 0.2f + 0.15f * std::sin(g.menuAnim * 1.5f + (x+y)*0.008f);
            drawRect(x + 23, y + 23, 3, 3, {60, 80, 160, (Uint8)(pulse*255)});
        }
    }

    // Title card
    int cardW = 500, cardH = 300;
    int cardX = (WINDOW_W - cardW) / 2;
    int cardY = (WINDOW_H - cardH) / 2 - 20;
    drawRect(cardX, cardY, cardW, cardH, {14, 14, 30, 230});

    // Border
    float bright = 0.5f + 0.5f * std::sin(g.menuAnim * 2);
    Uint8 bv = (Uint8)(bright * 160 + 40);
    setColor({bv, (Uint8)(bv/2), 255, 255});
    SDL_Rect border = {cardX, cardY, cardW, cardH};
    SDL_RenderDrawRect(g.renderer, &border);

    // Title area - "ZIELONY PŁYWAK" in green
    drawRect(cardX + 20, cardY + 30, cardW - 40, 4, {60, 200, 80, 200});
    drawRect(cardX + 20, cardY + 80, cardW - 40, 2, {60, 200, 80, 100});

    // Subtitle area
    drawRect(cardX + 20, cardY + 95, cardW - 40, 2, {96, 96, 255, 60});

    // Leaf decoration
    for (int i = 0; i < 5; i++) {
        float lx = cardX + 60 + i * 90;
        float ly = cardY + 55;
        float sway = 3 * std::sin(g.menuAnim * 2 + i);
        drawCircle((int)(lx + sway), (int)ly, 8, {40, 160, 50, 150});
    }

    // "Press ENTER to start" indicator
    float blink = 0.5f + 0.5f * std::sin(g.menuAnim * 3);
    drawRect(cardX + 120, cardY + 220, cardW - 240, 30,
             {96, 96, 255, (Uint8)(60 + blink*100)});
    drawRect(cardX + 122, cardY + 222, cardW - 244, 26,
             {14, 14, 30, 200});

    // Footer
    drawRect(0, WINDOW_H - 28, WINDOW_W, 28, {8, 8, 26, 200});
    drawRect(0, WINDOW_H - 28, WINDOW_W, 1, {96, 96, 255, 40});
}

void drawPaused() {
    // Dim overlay
    drawRect(0, 0, WINDOW_W, WINDOW_H, {0, 0, 0, 150});

    // Pause card
    int pw = 300, ph = 150;
    int px = (WINDOW_W - pw) / 2;
    int py = (WINDOW_H - ph) / 2;
    drawRect(px, py, pw, ph, {14, 14, 30, 240});
    setColor(COL_UI_ACCENT);
    SDL_Rect b = {px, py, pw, ph};
    SDL_RenderDrawRect(g.renderer, &b);

    // Pause bars
    drawRect(px + pw/2 - 20, py + 40, 15, 50, COL_UI_ACCENT);
    drawRect(px + pw/2 + 5, py + 40, 15, 50, COL_UI_ACCENT);

    // "ESC to resume" hint
    drawRect(px + 60, py + ph - 30, pw - 120, 20, {96, 96, 255, 60});
}

void drawWin() {
    // Render the game world behind
    drawSky();
    // CHANGED: TileMap renderer
    drawTileMap(g.renderer, g.currentZoneMap, (int)g.camera.x, (int)g.camera.y);
    drawTrees();
    drawDecorations();
    drawAnimals();
    drawPlayer();
    drawParticles();

    // Green overlay
    drawRect(0, 0, WINDOW_W, WINDOW_H, {20, 60, 20, 120});

    // Win card
    int cw = 500, ch = 250;
    int cx = (WINDOW_W - cw) / 2;
    int cy = (WINDOW_H - ch) / 2;
    drawRect(cx, cy, cw, ch, {14, 30, 14, 230});

    // Green border
    float pulse = 0.5f + 0.5f * std::sin(g.menuAnim * 3);
    setColor({60, (Uint8)(160 + pulse*95), 60, 255});
    SDL_Rect b = {cx, cy, cw, ch};
    SDL_RenderDrawRect(g.renderer, &b);

    // Eco 100% full bar
    drawRect(cx + 30, cy + 60, cw - 60, 20, {30, 50, 30, 200});
    drawRect(cx + 30, cy + 60, cw - 60, 20, COL_ECO_HIGH);

    // Star decorations
    for (int i = 0; i < 5; i++) {
        float sx = cx + 100 + i * 70;
        float sy = cy + 120;
        float twinkle = 0.5f + 0.5f * std::sin(g.menuAnim * 4 + i * 1.2f);
        int size = (int)(6 + twinkle * 4);
        drawCircle((int)sx, (int)sy, size, {255, 220, 60, (Uint8)(200 + twinkle*55)});
    }

    // "ENTER to return" blink
    float blink = 0.5f + 0.5f * std::sin(g.menuAnim * 3);
    drawRect(cx + 120, cy + 190, cw - 240, 30,
             {60, 200, 80, (Uint8)(60 + blink*120)});
}

void render() {
    switch (g.scene) {
        case GameScene::MENU:
            drawMenu();
            break;
        case GameScene::PLAYING:
            drawSky();
            // CHANGED: TileMap renderer replaces procedural drawTerrain()
            drawTileMap(g.renderer, g.currentZoneMap, (int)g.camera.x, (int)g.camera.y);
            drawTrees();
            drawDecorations();
            drawTrash();
            drawAnimals();
            drawPlayer();
            drawParticles();
            drawHUD();
            break;
        case GameScene::PAUSED:
            drawSky();
            // CHANGED: TileMap renderer replaces procedural drawTerrain()
            drawTileMap(g.renderer, g.currentZoneMap, (int)g.camera.x, (int)g.camera.y);
            drawTrees();
            drawDecorations();
            drawTrash();
            drawAnimals();
            drawPlayer();
            drawParticles();
            drawHUD();
            drawPaused();
            break;
        case GameScene::WIN:
            drawWin();
            break;
    }

    SDL_RenderPresent(g.renderer);
}

// ─── Main loop ───────────────────────────────────────────────────────────────

void mainLoopCallback() {
    Uint32 now = SDL_GetTicks();
    g.deltaTime = (now - g.lastTick) / 1000.0f;
    if (g.deltaTime > 0.05f) g.deltaTime = 0.05f;
    g.lastTick = now;
    g.totalTime += g.deltaTime;
    g.frameCount++;

    handleEvents();
    update(g.deltaTime);
    render();
}
