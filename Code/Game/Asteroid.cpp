#include "Asteroid.hpp"
#include "GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"

//-----------------------------------------------------------------------------------------------
Asteroid::Asteroid(Game* game)
	: Entity(game, Vec2(RandomNumberGenerator().RollRandomFloatInRange(0, WORLD_SIZE_X),
						RandomNumberGenerator().RollRandomFloatInRange(0, WORLD_SIZE_Y)))
{
	m_velocity = Vec2(RandomNumberGenerator().RollRandomFloatZeroToOne(), 
					  RandomNumberGenerator().RollRandomFloatZeroToOne()).GetNormalized();
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_angularVelocity = RandomNumberGenerator().RollRandomFloatInRange(-200.f,200.f);
	m_health = 3;

	// Define the asteroid mesh (in local space)
	for (int i = 0; i < 48; i += 3) {
		float randomLength = RandomNumberGenerator().RollRandomFloatInRange(ASTEROID_PHYSICS_RADIUS, 
																		  ASTEROID_COSMETIC_RADIUS);
		m_localMesh[i] = Vertex(Vec3(randomLength * CosDegrees((i + 1) / 16.f * 360.f),
									 randomLength * SinDegrees((i + 1) / 16.f * 360.f),
									 0.f),
								Rgba8(100, 100, 100, 255),
								Vec2(0.f, 0.f)
								);
		m_localMesh[i+1] = Vertex(Vec3(randomLength * CosDegrees(i / 16.f * 360.f),
								        randomLength * SinDegrees(i / 16.f * 360.f),
			                            0.f),
								  Rgba8(100, 100, 100, 255), 
								  Vec2(0.f, 0.f)
							      );
		m_localMesh[i + 2] = Vertex(Vec3(0, 0, 0), Rgba8(100, 100, 100, 255), Vec2(0.f, 0.f));
	}
}

//-----------------------------------------------------------------------------------------------
void Asteroid::Update(float deltaSeconds)
{
	m_position += m_velocity * ASTEROID_SPEED * deltaSeconds;
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
}

//-----------------------------------------------------------------------------------------------
void Asteroid::Render()
{
	for (int i = 0; i < 48; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
	}
	TransformVertexArrayXY3D(48, m_worldMesh, 1.f, m_orientationDegrees, m_position);
	g_engine->m_renderer->DrawVertexArray(48, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void Asteroid::Die()
{
	m_isDead = true;
}

