#pragma once
#include <string>
#include <vector>

// A single line of dialogue
struct DialogueLine {
    std::string speaker;   // "Marta", "Tomek", "Baba Sela", "Player"
    std::string text;      // The spoken line
    std::string condition; // optional: "marta_helped", "tomek_helped", etc. empty = always show
};

// A dialogue script = sequence of lines
struct DialogueScript {
    std::string id;   // e.g. "marta_intro", "marta_complete"
    std::vector<DialogueLine> lines;
};

struct DialogueManager {
    bool active = false;
    int currentLine = 0;
    DialogueScript* current = nullptr;
    // condition flags (set from game state)
    bool marta_helped = false;
    bool tomek_helped = false;
    bool sela_helped  = false;

    void start(DialogueScript* script) {
        if (!script || script->lines.empty()) return;
        current     = script;
        currentLine = 0;
        active      = true;
    }

    // Returns true if there are more lines
    bool advance() {
        if (!active) return false;
        currentLine++;
        if (currentLine >= (int)current->lines.size()) {
            active = false;
            current = nullptr;
            return false;
        }
        return true;
    }

    // Returns current speaker + text, empty strings if not active
    void current_line(std::string& speaker, std::string& text) {
        if (!active || !current) { speaker = ""; text = ""; return; }
        speaker = current->lines[currentLine].speaker;
        text    = current->lines[currentLine].text;
    }

    // Render dialogue box using SDL2 primitives (no TTF — drawn as coloured rectangles + placeholder)
    void render(SDL_Renderer* renderer, int screenW, int screenH) {
        if (!active) return;
        std::string speaker, text;
        current_line(speaker, text);

        // Box background
        SDL_Rect box = { 16, screenH - 100, screenW - 32, 84 };
        SDL_SetRenderDrawColor(renderer, 15, 20, 30, 220);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &box);

        // Border
        SDL_SetRenderDrawColor(renderer, 100, 200, 120, 255);
        SDL_RenderDrawRect(renderer, &box);

        // Speaker name block (green highlight)
        SDL_Rect nameBox = { 24, screenH - 108, (int)speaker.size() * 8 + 12, 14 };
        SDL_SetRenderDrawColor(renderer, 40, 120, 60, 255);
        SDL_RenderFillRect(renderer, &nameBox);

        // Prompt dot (blink indicator) bottom-right
        SDL_Rect prompt = { screenW - 32, screenH - 24, 8, 8 };
        SDL_SetRenderDrawColor(renderer, 100, 220, 140, 255);
        SDL_RenderFillRect(renderer, &prompt);
    }
};
