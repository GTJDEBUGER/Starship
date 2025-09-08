#include "Entity.hpp"
#include "GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game.hpp"

//-----------------------------------------------------------------------------------------------
Entity ::Entity(Game* game, Vec2 startPos)
	: m_game(game)
	, m_position(startPos)
	, m_velocity(Vec2(0.f,0.f))
	, m_orientationDegrees(0.f)
	, m_angularVelocity(0.f)
	, m_physicsRadius(1.f)
	, m_cosmeticRadius(1.f)
	, m_health(1)
	, m_isDead(false)
	, m_isGarbage(false) {
}

//-----------------------------------------------------------------------------------------------
Entity::~Entity() {
}

//-----------------------------------------------------------------------------------------------
bool Entity::IsOffScreen() const {
	if (m_position.x + m_cosmeticRadius < 0 || m_position.x - m_cosmeticRadius > WORLD_SIZE_X ||
		m_position.y + m_cosmeticRadius < 0 || m_position.y - m_cosmeticRadius > WORLD_SIZE_Y) {
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------
Vec2 Entity::GetForwardVector() const {
	return Vec2(CosDegrees(m_orientationDegrees), SinDegrees(m_orientationDegrees));
}