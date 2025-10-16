#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex.hpp"

class Game;

//-----------------------------------------------------------------------------------------------
class Entity {
public:
	virtual ~Entity()=0;
	Entity(Game* game, Vec2 startPos);

	virtual void          Update(float deltaSeconds)                     = 0;
	virtual void          Render() const                                 = 0;
	virtual void          Die()                                          = 0;
				          
	bool                  IsOffWorld() const;
	Vec2                  GetForwardVector() const;
				          
	void                  SetPositionRandomOffWorld();
	void                  SetPositionRandomOffScreen(Vec2 screenCenter);
	bool                  TeleportFromBoundary();
				          
public:			          
	Vec2                  m_position;
	Vec2                  m_velocity;
	float                 m_orientationDegrees;
	float                 m_angularVelocity;
	float                 m_physicsRadius;
	float                 m_cosmeticRadius;
	int                   m_health;
	bool                  m_isDead;
	bool                  m_isGarbage;
	Game*                 m_game                                         = nullptr;
	Vertex*               m_localMesh                                    = nullptr;
};