#include "hud.h"
#include <cmath>
#include <algorithm>

void drawHUD() {
    // Eco Meter (top-left)
    drawRect(16, 16, 204, 28, COL_UI_BG);
    drawRect(18, 18, 200, 24, {30, 30, 50, 200});

    // Eco bar
    float eco01 = g.ecoMeter / 100.0f;
    int barW = (int)(196 * eco01);
    Color barCol;
    if (eco01 < 0.33f) barCol = lerpColor(COL_ECO_LOW, COL_ECO_MID, eco01 / 0.33f);
    else if (eco01 < 0.66f) barCol = lerpColor(COL_ECO_MID, COL_ECO_HIGH, (eco01-0.33f)/0.33f);
    else barCol = COL_ECO_HIGH;
    drawRect(20, 20, barW, 20, barCol);

    // Eco label "ECO" - pixel text
    drawRect(24, 24, 2, 12, COL_WHITE); // E
    drawRect(24, 24, 8, 2, COL_WHITE);
    drawRect(24, 29, 6, 2, COL_WHITE);
    drawRect(24, 34, 8, 2, COL_WHITE);

    drawRect(38, 24, 2, 12, COL_WHITE); // C
    drawRect(38, 24, 8, 2, COL_WHITE);
    drawRect(38, 34, 8, 2, COL_WHITE);

    drawRect(52, 24, 2, 12, COL_WHITE); // O
    drawRect(52, 24, 8, 2, COL_WHITE);
    drawRect(52, 34, 8, 2, COL_WHITE);
    drawRect(58, 24, 2, 12, COL_WHITE);

    // Trash counter (top-right area)
    int cx = WINDOW_W - 120;
    drawRect(cx - 4, 16, 108, 28, COL_UI_BG);

    // Trash icon (small bag)
    drawCircle(cx + 8, 30, 6, COL_TRASH);

    // Counter digits - crude but functional
    // Show as filled squares proportional to progress
    int collected = g.player.trashCollected;
    int total = g.player.totalTrash;
    float progress = total > 0 ? (float)collected / total : 0;
    int fillW = (int)(80 * progress);
    drawRect(cx + 20, 22, 80, 16, {30, 30, 50, 200});
    drawRect(cx + 20, 22, fillW, 16, {60, 200, 80, 200});

    // Zone name display
    if (g.zoneNameTimer > 0) {
        float alpha = std::min(1.0f, g.zoneNameTimer);
        if (g.zoneNameTimer < 0.5f) alpha = g.zoneNameTimer * 2;
        int nameW = 300;
        int nameX = (WINDOW_W - nameW) / 2;
        int nameY = 80;
        drawRect(nameX, nameY, nameW, 36, {10, 10, 26, (Uint8)(200*alpha)});
        // Zone indicator line
        drawRect(nameX, nameY + 34, nameW, 2, {96, 96, 255, (Uint8)(200*alpha)});
    }

    // Popup message
    if (g.popupTimer > 0) {
        float alpha = std::min(1.0f, g.popupTimer);
        int pw = 350;
        int px = (WINDOW_W - pw) / 2;
        int py = WINDOW_H - 100;
        drawRect(px, py, pw, 32, {10, 30, 10, (Uint8)(200*alpha)});
        drawRect(px, py, 4, 32, {60, 200, 80, (Uint8)(255*alpha)});
    }

    // Controls hint (bottom-left, fades out)
    float hintAlpha = std::max(0.0f, 1.0f - g.totalTime * 0.1f);
    if (hintAlpha > 0) {
        drawRect(16, WINDOW_H - 48, 300, 32, {10, 10, 26, (Uint8)(180*hintAlpha)});
        // Arrow/WASD hint line
        drawRect(20, WINDOW_H - 44, 2, 2, {96, 96, 255, (Uint8)(200*hintAlpha)});
        drawRect(24, WINDOW_H - 44, 30, 2, {200, 200, 240, (Uint8)(150*hintAlpha)});
    }

    // Mini-map (bottom-right)
    int mmX = WINDOW_W - 170;
    int mmY = WINDOW_H - 55;
    int mmW = 154;
    int mmH = 40;
    drawRect(mmX - 2, mmY - 2, mmW + 4, mmH + 4, COL_UI_BG);

    // Zone colors on minimap
    int beachW = (int)(mmW * (float)ZONE_BEACH_END / WORLD_W);
    int forestW = (int)(mmW * (float)(ZONE_FOREST_END - ZONE_FOREST_START) / WORLD_W);
    int wetW = mmW - beachW - forestW;
    drawRect(mmX, mmY, beachW, mmH, {220, 200, 160, 100});
    drawRect(mmX + beachW, mmY, forestW, mmH, {60, 160, 60, 100});
    drawRect(mmX + beachW + forestW, mmY, wetW, mmH, {100, 80, 50, 100});

    // Trash dots on minimap
    for (auto& t : g.trash) {
        if (t.collected) continue;
        int tx = mmX + (int)(t.pos.x / WORLD_W * mmW);
        int ty = mmY + mmH/2;
        drawRect(tx, ty, 2, 2, COL_TRASH);
    }

    // Player dot on minimap
    int plx = mmX + (int)(g.player.pos.x / WORLD_W * mmW);
    drawRect(plx - 1, mmY + mmH/2 - 1, 4, 4, COL_PLAYER_BODY);

    // Camera view indicator
    int cvx = mmX + (int)(g.camera.x / WORLD_W * mmW);
    int cvw = (int)((float)WINDOW_W / WORLD_W * mmW);
    setColor({96, 96, 255, 80});
    SDL_Rect cvr = {cvx, mmY, cvw, mmH};
    SDL_RenderFillRect(g.renderer, &cvr);
    SDL_SetRenderDrawColor(g.renderer, 96, 96, 255, 150);
    SDL_RenderDrawRect(g.renderer, &cvr);

    // DEBUG: camera.x progress bar (bottom of screen, 4px tall)
    // Shows scroll position from 0 (left) to WORLD_W-WINDOW_W (right)
    {
        const int maxScroll = WORLD_W - WINDOW_W; // 2560
        float camPct = (maxScroll > 0) ? std::min(1.0f, g.camera.x / (float)maxScroll) : 0.0f;
        int barW2 = (int)(camPct * WINDOW_W);
        // Dark bg
        drawRect(0, WINDOW_H - 6, WINDOW_W, 6, {10, 10, 26, 180});
        // Progress fill — green at start, yellow at mid, orange near end
        Color dbgCol = {50, 220, 100, 200};
        if (camPct > 0.66f) dbgCol = {220, 160, 50, 200};
        else if (camPct > 0.33f) dbgCol = {180, 220, 50, 200};
        drawRect(0, WINDOW_H - 6, barW2, 6, dbgCol);
        // Tick at start and end
        drawRect(0, WINDOW_H - 6, 2, 6, COL_WHITE);
        drawRect(WINDOW_W - 2, WINDOW_H - 6, 2, 6, COL_WHITE);
    }
}
