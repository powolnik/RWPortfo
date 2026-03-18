#include "player.h"
#include <cmath>

// ── Placeholder 16-frame player sprite sheet (256×32) ────────────────────────
// 16 frames × 16px wide × 32px tall. Each frame is a coloured rectangle with
// a small white marker so you can see which frame is active.
SDL_Texture* createPlaceholderPlayerSheet(SDL_Renderer* renderer) {
    const int FW = 16, FH = 32, FRAMES = 16;
    SDL_Texture* tex = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_TARGET,
                                         FW * FRAMES, FH);
    if (!tex) return nullptr;
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, tex);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for (int i = 0; i < FRAMES; i++) {
        int ox = i * FW;
        // Body: green with slight hue shift per frame for visibility
        Uint8 g_val = (Uint8)(140 + i * 6);
        SDL_SetRenderDrawColor(renderer, 30, g_val, 60, 255);
        SDL_Rect body = {ox + 3, 8, 10, 20};
        SDL_RenderFillRect(renderer, &body);

        // Head
        SDL_SetRenderDrawColor(renderer, 210, 160, 110, 255);
        SDL_Rect head = {ox + 4, 1, 8, 8};
        SDL_RenderFillRect(renderer, &head);

        // Legs
        SDL_SetRenderDrawColor(renderer, 20, 80, 40, 255);
        SDL_Rect legs = {ox + 3, 28, 10, 4};
        SDL_RenderFillRect(renderer, &legs);

        // White dot — frame indicator (top-left corner of frame)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
        SDL_Rect dot = {ox + 1, 1, 2, 2};
        SDL_RenderFillRect(renderer, &dot);
        // Extra dots for frame index (binary-ish visual)
        if (i & 1) { SDL_Rect d2 = {ox+1, 4, 2, 2}; SDL_RenderFillRect(renderer, &d2); }
        if (i & 2) { SDL_Rect d3 = {ox+4, 1, 2, 2}; SDL_RenderFillRect(renderer, &d3); }
        if (i & 4) { SDL_Rect d4 = {ox+4, 4, 2, 2}; SDL_RenderFillRect(renderer, &d4); }
    }

    SDL_SetRenderTarget(renderer, nullptr);
    return tex;
}

void updatePlayer(float dt) {
    Player& p = g.player;
    p.animTime += dt;
    p.pickupCooldown -= dt;
    if (p.pickupCooldown < 0) p.pickupCooldown = 0;

    // ── Drive AnimationController ──────────────────────────────────────────
    {
        const std::string prevClip = p.anim.currentClip;
        if (p.pickupCooldown > 0.01f) {
            playClip(p.anim, "pickup");
        } else if (!p.onGround && !p.inWater) {
            playClip(p.anim, "jump");
        } else if (std::abs(p.vel.x) > 10.0f) {
            playClip(p.anim, "walk");
        } else {
            playClip(p.anim, "idle");
        }
        updateAnimation(p.anim, dt);
    }

    // Horizontal movement
    float moveX = 0;
    if (g.input.left)  moveX -= 1.0f;
    if (g.input.right) moveX += 1.0f;

    if (moveX != 0) {
        p.facingRight = moveX > 0;
    }

    float speed = p.inWater ? SWIM_SPEED : PLAYER_SPEED;
    p.vel.x = moveX * speed;

    // Vertical
    if (p.inWater) {
        // Swimming
        p.vel.y *= 0.92f; // water drag
        if (g.input.up)   p.vel.y -= 400.0f * dt;
        if (g.input.down)  p.vel.y += 200.0f * dt;
        p.vel.y += GRAVITY * 0.15f * dt; // slight sink
        // Clamp vertical speed in water
        if (p.vel.y > 120.0f) p.vel.y = 120.0f;
        if (p.vel.y < -180.0f) p.vel.y = -180.0f;
    } else {
        // Gravity
        p.vel.y += GRAVITY * dt;
        // Jump
        if (g.input.jumpPressed && p.onGround) {
            p.vel.y = -JUMP_FORCE;
            p.onGround = false;
        }
    }

    // Apply velocity
    p.pos.x += p.vel.x * dt;
    p.pos.y += p.vel.y * dt;

    // World bounds
    if (p.pos.x < 20) p.pos.x = 20;
    if (p.pos.x > WORLD_W - 20) p.pos.x = WORLD_W - 20;

    // CHANGED: Ground collision via TileMap (replaces getGroundY heightmap)
    p.onGround = false;
    float feetY  = p.pos.y + p.h / 2.0f;
    int   tileTs = g.currentZoneMap.tileSize;
    bool  hitGround =
        isSolidTile(g.currentZoneMap, (int)p.pos.x,               (int)feetY) ||
        isSolidTile(g.currentZoneMap, (int)(p.pos.x - p.w/2 + 2), (int)feetY) ||
        isSolidTile(g.currentZoneMap, (int)(p.pos.x + p.w/2 - 2), (int)feetY);
    if (hitGround && p.vel.y >= 0) {
        // Snap feet to top edge of the tile row they entered
        int tileRow  = (int)feetY / tileTs;
        p.pos.y = (float)(tileRow * tileTs) - p.h / 2.0f;
        p.vel.y = 0;
        p.onGround = true;
    }

    // CHANGED: Water check via TileMap (replaces isWaterAt)
    p.inWater = isWaterTile(g.currentZoneMap, (int)p.pos.x, (int)(p.pos.y));

    // Clamp camera
    if (g.camera.x < 0) g.camera.x = 0;
    if (g.camera.x > WORLD_W - WINDOW_W) g.camera.x = WORLD_W - WINDOW_W;
    if (g.camera.y < 0) g.camera.y = 0;
    if (g.camera.y > WORLD_H - WINDOW_H) g.camera.y = WORLD_H - WINDOW_H;
}

void drawPlayer() {
    Player& p = g.player;
    float sx = p.pos.x - g.camera.x;
    float sy = p.pos.y - g.camera.y;
    float dir = p.facingRight ? 1.0f : -1.0f;

    // ── If a sprite sheet is available, use it ────────────────────────────
    if (g.playerSheet.texture) {
        int frame = getCurrentFrameIndex(p.anim);
        // Centre the 32×64 drawn sprite on the player's logical position
        int drawX = (int)sx - 16;  // 16px × scale2 = 32px wide → offset 16
        int drawY = (int)sy - 48;  // 32px × scale2 = 64px tall → feet at sy+16
        drawFrame(g.renderer, g.playerSheet, frame, drawX, drawY,
                  !p.facingRight, 2);
        return;
    }

    // ── Fallback: SDL2 primitive drawing (no sprite sheet loaded) ─────────
    // Walking animation
    float legAnim = p.onGround && std::abs(p.vel.x) > 10 ?
                    std::sin(p.animTime * 10) * 6 : 0;
    float armAnim = legAnim * 0.7f;

    // Swimming animation
    if (p.inWater) {
        legAnim = std::sin(p.animTime * 4) * 4;
        armAnim = std::sin(p.animTime * 4 + 1.5f) * 8;
    }

    // Legs
    setColor({40, 100, 60, 255});
    SDL_RenderDrawLine(g.renderer, (int)sx, (int)(sy+8),
                       (int)(sx - 5*dir + legAnim), (int)(sy + 22));
    SDL_RenderDrawLine(g.renderer, (int)sx, (int)(sy+8),
                       (int)(sx + 5*dir - legAnim), (int)(sy + 22));

    // Body (green outfit - eco!)
    drawRect((int)sx-8, (int)sy-12, 16, 24, COL_PLAYER_BODY);

    // Arms
    setColor(COL_PLAYER_BODY);
    SDL_RenderDrawLine(g.renderer, (int)(sx - 8), (int)(sy - 4),
                       (int)(sx - 14 + armAnim), (int)(sy + 8));
    SDL_RenderDrawLine(g.renderer, (int)(sx + 8), (int)(sy - 4),
                       (int)(sx + 14 - armAnim), (int)(sy + 8));

    // Head
    drawCircle((int)sx, (int)(sy - 18), 9, COL_PLAYER_HEAD);

    // Hair
    drawRect((int)sx-8, (int)(sy-26), 16, 6, {60, 40, 20, 255});

    // Eyes
    int eyeOff = (int)(3 * dir);
    drawCircle((int)(sx + eyeOff - 2), (int)(sy - 19), 2, COL_WHITE);
    drawCircle((int)(sx + eyeOff + 2), (int)(sy - 19), 2, COL_WHITE);
    drawCircle((int)(sx + eyeOff - 2), (int)(sy - 19), 1, COL_BLACK);
    drawCircle((int)(sx + eyeOff + 2), (int)(sy - 19), 1, COL_BLACK);

    // Backpack (for collected trash)
    float bpDir = -dir;
    drawRect((int)(sx + 9*bpDir), (int)(sy - 10), 8, 16, {80, 60, 40, 255});
    drawRect((int)(sx + 10*bpDir), (int)(sy - 8), 6, 3, {100, 80, 50, 255});
}
