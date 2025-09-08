#pragma once
#include "Engine/Math/Vec2.hpp"

class Game;

//-----------------------------------------------------------------------------------------------
class Entity {
public:
	virtual ~Entity()=0;
	Entity(Game* game, Vec2 startPos);

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() = 0;
	virtual void Die() = 0;

	bool IsOffScreen() const;
	Vec2 GetForwardVector() const;

public:
	Vec2 m_position;
	Vec2 m_velocity;
	float m_orientationDegrees;
	float m_angularVelocity;
	float m_physicsRadius;
	float m_cosmeticRadius;
	int m_health;
	bool m_isDead;
	bool m_isGarbage;
	Game* m_game = nullptr;

private:
	Vec2 lastFramePosition;
};