#include "Entity.hpp"
#include "GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Game.hpp"

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
	, m_isGarbage(false)
	, m_localMesh(nullptr){
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

//-----------------------------------------------------------------------------------------------
void Entity::SetPositionRandomOffWorld() {
	if (m_randomGenerator.RollRandomFloatZeroToOne() < 0.25f) {
		m_position.x = 0 - m_cosmeticRadius;
		m_position.y = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, WORLD_SIZE_Y + m_cosmeticRadius);
	}
	else if (m_randomGenerator.RollRandomFloatZeroToOne() >= 0.25f &&
		m_randomGenerator.RollRandomFloatZeroToOne() < 0.5f) {
		m_position.x = WORLD_SIZE_X + m_cosmeticRadius;
		m_position.y = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, WORLD_SIZE_Y + m_cosmeticRadius);
	}
	else if (m_randomGenerator.RollRandomFloatZeroToOne() >= 0.5f &&
		m_randomGenerator.RollRandomFloatZeroToOne() < 0.75f) {
		m_position.y = 0 - m_cosmeticRadius;
		m_position.x = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, WORLD_SIZE_X + m_cosmeticRadius);
	}
	else {
		m_position.y = WORLD_SIZE_Y + m_cosmeticRadius;
		m_position.x = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, WORLD_SIZE_X + m_cosmeticRadius);
	}
}

//-----------------------------------------------------------------------------------------------
void Entity::SetPositionRandomOffScreen(Vec2 screenCenter) {
	if (m_randomGenerator.RollRandomFloatZeroToOne() < 0.25f) {
		m_position.x = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, screenCenter.x - VIEW_CENTER_X - m_cosmeticRadius);
		m_position.y = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, WORLD_SIZE_Y + m_cosmeticRadius);
	}
	else if (m_randomGenerator.RollRandomFloatZeroToOne() >= 0.25f &&
		m_randomGenerator.RollRandomFloatZeroToOne() < 0.5f) {
		m_position.x = m_randomGenerator.RollRandomFloatInRange(screenCenter.x + VIEW_CENTER_X + m_cosmeticRadius, WORLD_SIZE_X + m_cosmeticRadius);
		m_position.y = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, WORLD_SIZE_Y + m_cosmeticRadius);
	}
	else if (m_randomGenerator.RollRandomFloatZeroToOne() >= 0.5f &&
		m_randomGenerator.RollRandomFloatZeroToOne() < 0.75f) {
		m_position.y = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, screenCenter.y - VIEW_CENTER_Y - m_cosmeticRadius);
		m_position.x = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, WORLD_SIZE_X + m_cosmeticRadius);
	}
	else {
		m_position.y = m_randomGenerator.RollRandomFloatInRange(screenCenter.y + VIEW_CENTER_Y + m_cosmeticRadius, WORLD_SIZE_Y + m_cosmeticRadius);
		m_position.x = m_randomGenerator.RollRandomFloatInRange(-m_cosmeticRadius, WORLD_SIZE_X + m_cosmeticRadius);
	}
}


//----------------------------------------------------------------------------------------------
bool Entity::BoundaryTeleport() {
	if (m_position.x <= 0) {
		m_position.x = WORLD_SIZE_X;
		return true;
	}
	
	if (m_position.x >= WORLD_SIZE_X) {
		m_position.x = 0;
		return true;
	}

	if (m_position.y <= 0) {
		m_position.y = WORLD_SIZE_Y;
		return true;
	}
	
	if (m_position.y >= WORLD_SIZE_Y) {
		m_position.y = 0;
		return true;
	}

	return false;
}
