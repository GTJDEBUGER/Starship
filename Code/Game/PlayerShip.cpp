#define WIN32_LEAN_AND_MEAN		
#include <windows.h>
#include <gl/gl.h>
#pragma comment( lib, "opengl32" )

#include "PlayerShip.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
PlayerShip::PlayerShip(Vec2 const& startingPosition, Vec2 const& startingVelocity)
	: m_position(startingPosition)
	, m_velocity(startingVelocity)
{
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Update(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Render()
{
	glColor4ub(0, 0, 0, 255);
	glTexCoord2f(0.f, 0.f);
	glVertex2f(m_position.x - 2.f, m_position.y - 2.f);

	glColor4ub(255, 255, 255, 255);
	glTexCoord2f(0.f, 0.f);
	glVertex2f(m_position.x + 4.f, m_position.y);

	glColor4ub(0, 127, 255, 255);
	glTexCoord2f(0.f, 0.f);
	glVertex2f(m_position.x - 2.f, m_position.y + 2.f);
}