#include "Game/ShockWave.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Debris.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"

//-----------------------------------------------------------------------------------------------
ShockWave::ShockWave(Game* game, Vec2 position, float duration, float maxSpreadDistance, Rgba8 waveColor)
	: Entity(game, position)
{
	m_physicsRadius = 0.f;
	m_cosmeticRadius = 0.f;
	m_waveColor = waveColor;

	m_duration = duration;
	m_lifeTime = duration;
	m_spreadDistance = maxSpreadDistance;

	m_localMesh = new Vertex[m_vertexNum];
	for (int i = 0; i < 45; i += 3) {
		m_localMesh[i] = Vertex(Vec3(CosDegrees(((float)i / 3.f + 1.f) / 16.f * 360.f),
			SinDegrees(((float)i / 3.f + 1.f) / 16.f * 360.f),
			0.f),
			m_waveColor,
			Vec2(0.f, 0.f)
		);
		m_localMesh[i + 1] = Vertex(Vec3(CosDegrees(((float)i / 3.f) / 16.f * 360.f),
			SinDegrees(((float)i / 3.f) / 16.f * 360.f),
			0.f),
			m_waveColor,
			Vec2(0.f, 0.f)
		);
		m_localMesh[i + 2] = Vertex(Vec3(0.f, 0.f, 0.f),
			Rgba8(0,0,0,0),
			Vec2(0.f, 0.f)
		);
	}


	m_localMesh[45] = Vertex(Vec3(1.f, 0.f, 0.f),
		m_waveColor,
		Vec2(0.f, 0.f)
	);
	m_localMesh[46] = Vertex(Vec3(CosDegrees(15.f / 16.f * 360.f),
		SinDegrees(15.f / 16.f * 360.f),
		0.f),
		m_waveColor,
		Vec2(0.f, 0.f)
	);
	m_localMesh[47] = Vertex(Vec3(0.f, 0.f, 0.f),
		Rgba8(0,0,0,0),
		Vec2(0.f, 0.f)
	);
}

//-----------------------------------------------------------------------------------------------
void ShockWave::Update()
{
	m_lifeTime -= (float)m_game->m_gameClock->GetDeltaSeconds();
	if(m_lifeTime <= 0.f) {
		Die();
		return;
	}
}

//-----------------------------------------------------------------------------------------------
void ShockWave::Render() const
{
	Vertex m_worldMesh[m_vertexNum];
	for (int i = 0; i < m_vertexNum; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
		m_worldMesh[i].m_color = Rgba8(m_worldMesh[i].m_color.r, 
									   m_worldMesh[i].m_color.g, 
									   m_worldMesh[i].m_color.b, 
									   (unsigned char) (255.f * m_lifeTime/m_duration));
	}
	TransformVertexArrayXY3D(m_vertexNum, m_worldMesh, m_spreadDistance * (1.f - m_lifeTime/m_duration), 0.f, m_position);
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray(m_vertexNum, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void ShockWave::Die()
{
	if (!m_isDead) {
		m_isDead = true;
	}
}