#pragma once
constexpr int NUM_STARTING_ASTEROIDS = 6;
constexpr int MAX_ASTEROIDS = 12;
constexpr int MAX_BULLETS = 20;
constexpr int MAX_BEETLES = 10;
constexpr int DEBUG_DRAWRING_SUBDIVISION = 32;
constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
constexpr float ASTEROID_SPEED = 10.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
constexpr float BULLET_LIFETIME_SECONDS = 2.0f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;
constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr float ENEMY_BEETLE_SPEED = 9.f;
constexpr float ENEMY_BEETLE_PHYSICS_RADIUS = 1.75f;
constexpr float ENEMY_BEETLE_COSMETIC_RADIUS = 2.25f;

struct Vec2;
struct Rgba8;

void DebugDrawLine(Vec2 startPosition, Vec2 endPosition, Rgba8 lineCOlor, float thickness);
void DebugDrawRing(Vec2 centre, float radius, Rgba8 ringColor, float thickness);
