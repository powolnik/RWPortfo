#pragma once
/**
 * economy.h — Creator Credits loop for Zielony Pływak
 *
 * Phase 2: LitterType classification, credit awards, inventory,
 * burden penalty, recycle stations, and Tool Library.
 */

#include <string>
#include <vector>

// ─── Litter Classification ────────────────────────────────────────────────────

enum class LitterType {
    PLASTIC,    // +1 cr  — bottles, cans, bags
    ORGANIC,    // +2 cr  — composted on recycle
    CHEMICAL,   // +3 cr  — barrels
    MACHINERY,  // +3 cr  — tires, heavy debris
};

// Returns the credit award for a given litter type (before recycling)
inline int creditsForLitter(LitterType lt) {
    switch (lt) {
        case LitterType::PLASTIC:   return 1;
        case LitterType::ORGANIC:   return 2;
        case LitterType::CHEMICAL:  return 3;
        case LitterType::MACHINERY: return 3;
    }
    return 1;
}

inline const char* litterTypeName(LitterType lt) {
    switch (lt) {
        case LitterType::PLASTIC:   return "Plastic";
        case LitterType::ORGANIC:   return "Organic";
        case LitterType::CHEMICAL:  return "Chemical";
        case LitterType::MACHINERY: return "Machinery";
    }
    return "Litter";
}

// ─── Tools ───────────────────────────────────────────────────────────────────

enum class ToolType {
    NONE,
    HAND_TROWEL,    //  0 cr — always available; picks up PLASTIC + ORGANIC
    PRUNING_SAW,    //  3 cr — picks up ORGANIC + small MACHINERY
    DRONE_CAMERA,   //  5 cr — enables photo atlas [F]; required for wildlife photos
    BIO_FILTER_KIT, //  8 cr — picks up CHEMICAL waste
    CRANE_ARM,      // 12 cr — picks up heavy MACHINERY
};

struct Tool {
    ToolType   type;
    const char* name;
    int         cost;          // Creator Credits to borrow
    const char* description;
    // Which litter types this tool can pick up (bit flags via LitterType)
    bool canPickupPlastic;
    bool canPickupOrganic;
    bool canPickupChemical;
    bool canPickupMachinery;
};

inline constexpr Tool TOOL_CATALOGUE[] = {
    // type                 name               cost  desc                                P      O      C      M
    { ToolType::NONE,            "Bare hands",      0,    "Can't pick up anything!",         false, false, false, false },
    { ToolType::HAND_TROWEL,     "Hand trowel",     0,    "Picks up plastic & organic",      true,  true,  false, false },
    { ToolType::PRUNING_SAW,     "Pruning saw",     3,    "Picks up organic & small debris", false, true,  false, true  },
    { ToolType::DRONE_CAMERA,    "Drone camera",    5,    "Enables wildlife photography",    true,  true,  false, false },
    { ToolType::BIO_FILTER_KIT,  "Bio-filter kit",  8,    "Neutralises chemical waste",      true,  true,  true,  false },
    { ToolType::CRANE_ARM,       "Crane arm",       12,   "Dismantles heavy machinery",      false, false, false, true  },
};

inline bool toolCanPickup(ToolType tool, LitterType litter) {
    for (auto& t : TOOL_CATALOGUE) {
        if (t.type != tool) continue;
        switch (litter) {
            case LitterType::PLASTIC:   return t.canPickupPlastic;
            case LitterType::ORGANIC:   return t.canPickupOrganic;
            case LitterType::CHEMICAL:  return t.canPickupChemical;
            case LitterType::MACHINERY: return t.canPickupMachinery;
        }
    }
    return false;
}

inline const Tool* getToolInfo(ToolType tool) {
    for (auto& t : TOOL_CATALOGUE) {
        if (t.type == tool) return &t;
    }
    return &TOOL_CATALOGUE[0];
}

// ─── Inventory ────────────────────────────────────────────────────────────────

constexpr int MAX_CARRY = 10;

struct Inventory {
    int carriedCount  = 0;     // items currently carried (not yet recycled)
    int credits       = 0;     // total Creator Credits earned
    ToolType heldTool = ToolType::HAND_TROWEL;  // tool currently borrowed
    bool hasBurden    = false; // true when carriedCount > MAX_CARRY
    int organicBonusCredits = 2; // default +2 per organic; upgraded to +3 by Tomek

    // Carried litter breakdown (for recycle payout calculation)
    int plasticHeld   = 0;
    int organicHeld   = 0;
    int chemicalHeld  = 0;
    int machineryHeld = 0;

    void addItem(LitterType lt) {
        carriedCount++;
        switch (lt) {
            case LitterType::PLASTIC:   plasticHeld++;   break;
            case LitterType::ORGANIC:   organicHeld++;   break;
            case LitterType::CHEMICAL:  chemicalHeld++;  break;
            case LitterType::MACHINERY: machineryHeld++; break;
        }
        hasBurden = (carriedCount > MAX_CARRY);
    }

    // Dump all carried items at recycle station → return credits earned
    int recycle(bool organicUpgrade = false) {
        int earned = 0;
        earned += plasticHeld   * creditsForLitter(LitterType::PLASTIC);
        earned += organicBonusCredits * organicHeld;   // Tomek upgrade: +3 instead of +2 (via organicBonusCredits)
        earned += chemicalHeld  * creditsForLitter(LitterType::CHEMICAL);
        earned += machineryHeld * creditsForLitter(LitterType::MACHINERY);
        credits += earned;
        // Clear held items
        carriedCount  = 0;
        plasticHeld   = 0;
        organicHeld   = 0;
        chemicalHeld  = 0;
        machineryHeld = 0;
        hasBurden     = false;
        return earned;
    }
};

// ─── Recycle Station ─────────────────────────────────────────────────────────

struct RecycleStation {
    float x, y;       // world position (centre)
    int   zoneIdx;    // which zone this station belongs to
    float w = 48, h = 56; // visual size

    bool playerInRange(float px, float py, float range = 60.0f) const {
        float dx = px - x;
        float dy = py - y;
        return (dx*dx + dy*dy) < range*range;
    }
};

// ─── Tool Library Station ────────────────────────────────────────────────────

struct ToolLibraryStation {
    float x, y;
    int   zoneIdx;    // only one station (Beach zone)
    float w = 48, h = 64;

    bool playerInRange(float px, float py, float range = 70.0f) const {
        float dx = px - x;
        float dy = py - y;
        return (dx*dx + dy*dy) < range*range;
    }
};

// ─── Tool Library UI State ───────────────────────────────────────────────────

struct ToolLibraryUI {
    bool     open       = false;
    int      selection  = 1;   // index into TOOL_CATALOGUE (0 = none, skip)
    float    openAnim   = 0;   // 0→1 open transition
};
