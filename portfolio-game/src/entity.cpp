#include "entity.h"
#include "spritesheet.h"

void renderEntity(SDL_Renderer* r, const Entity& e, int camX, int camY) {
    if (!e.active) return;
    int sx = (int)e.x - camX;
    int sy = (int)e.y - camY;
    if (e.sheet && e.sheet->texture) {
        int frame = getCurrentFrameIndex(e.anim);
        drawFrame(r, *e.sheet, frame, sx - e.width, sy - e.height, false, 2);
    } else {
        // Fallback: solid green rect
        SDL_SetRenderDrawColor(r, 0, 200, 80, 255);
        SDL_Rect rect = {sx - e.width/2, sy - e.height/2, e.width, e.height};
        SDL_RenderFillRect(r, &rect);
    }
}
