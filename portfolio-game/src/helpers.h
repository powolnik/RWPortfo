#pragma once
#include "types.h"

// ─── Drawing Helpers ─────────────────────────────────────────────────────────

void setColor(Color c);
void drawRect(int x, int y, int w, int h, Color c);
void drawRectWorld(float wx, float wy, int w, int h, Color c);
void drawCircle(int cx, int cy, int radius, Color c);
void drawCircleWorld(float wx, float wy, int radius, Color c);

float lerp(float a, float b, float t);
Color lerpColor(Color a, Color b, float t);

float randf();
float randf(float lo, float hi);

// ─── Terrain Query Helpers ───────────────────────────────────────────────────

int getGroundY(float worldX);
bool isWaterAt(float worldX, float worldY);
int getWaterLevel(float worldX);
Zone getZone(float worldX);
const char* getZoneName(Zone z);
