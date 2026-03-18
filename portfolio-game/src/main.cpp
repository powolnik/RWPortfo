/**
 * Zielony Pływak (The Green Floater)
 * C++ / SDL2 / Emscripten → WebAssembly
 *
 * Entry point only — all game logic lives in the other modules.
 * Build: see Makefile
 */

#include <SDL2/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "game.h"

// ─── Entry Point ─────────────────────────────────────────────────────────────

int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    g.window = SDL_CreateWindow(
        "Zielony Plywak",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );
    if (!g.window) { SDL_Log("CreateWindow: %s", SDL_GetError()); return 1; }

    g.renderer = SDL_CreateRenderer(
        g.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!g.renderer) { SDL_Log("CreateRenderer: %s", SDL_GetError()); return 1; }

    SDL_SetRenderDrawBlendMode(g.renderer, SDL_BLENDMODE_BLEND);
    g.lastTick = SDL_GetTicks();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoopCallback, 0, 1);
#else
    bool running = true;
    while (running) {
        mainLoopCallback();
        SDL_Delay(1);
    }
#endif

    SDL_DestroyRenderer(g.renderer);
    SDL_DestroyWindow(g.window);
    SDL_Quit();
    return 0;
}
