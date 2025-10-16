#include "Game/Debris.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
Debris::Debris(Game* game, Vec2 burstPosition, Vec2 burstDirection, float burstSpeed, Rgba8 color, float meshScale)
	:Entity(game,burstPosition) {
	m_velocity = burstDirection * burstSpeed;
	m_orientationDegrees = burstDirection.GetOrientationDegrees();
	m_physicsRadius = DEBRIS_PHYSICS_RADIUS;
	m_cosmeticRadius = DEBRIS_COSMETIC_RADIUS;
	m_lifeTime = 2.f;
	m_lifeTimeCur = m_lifeTime;
	m_meshScale = meshScale;

	m_localMesh = new Vertex[m_vertexNum];
	GetLocalMesh(m_vertexNum, m_localMesh, color);
}

//-----------------------------------------------------------------------------------------------
void Debris::Update(float deltaSeconds)
{
	m_lifeTimeCur -= deltaSeconds;
	if (m_lifeTimeCur <= 0.f) {
		Die();
		return;
	}
	SetColorDuringLifeTime();
	m_position += m_velocity * deltaSeconds;
}

//-----------------------------------------------------------------------------------------------
void Debris::Render() const
{
	Vertex m_worldMesh[MAX_DEBRIS_SUBDIVISION * 3];
	for (int i = 0; i < MAX_DEBRIS_SUBDIVISION * 3; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
	}
	TransformVertexArrayXY3D(MAX_DEBRIS_SUBDIVISION * 3, m_worldMesh, m_meshScale, m_orientationDegrees, m_position);
	g_engine->m_renderer->DrawVertexArray(MAX_DEBRIS_SUBDIVISION * 3, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void Debris::Die()
{
	m_isDead = true;
}

//-----------------------------------------------------------------------------------------------
void Debris::GetLocalMesh(int vertexNum, Vertex* mesh, Rgba8 color) {
	GUARANTEE_OR_DIE(vertexNum == m_vertexNum, "The array you provided can not save debris mesh!");

	float randomLengths[MAX_DEBRIS_SUBDIVISION];
	for (int i = 0; i < MAX_DEBRIS_SUBDIVISION; i++) {
		randomLengths[i] = RandomNumberGenerator().RollRandomFloatInRange(DEBRIS_PHYSICS_RADIUS,
			DEBRIS_COSMETIC_RADIUS);
	}
	// define the debris mesh (in local space)
	for (int i = 0; i < MAX_DEBRIS_SUBDIVISION * 3 - 3; i += 3) {
		mesh[i] = Vertex(Vec3(randomLengths[i / 3 + 1] * CosDegrees((i / 3 + 1) / static_cast<float>(MAX_DEBRIS_SUBDIVISION) * 360.f),
			randomLengths[i / 3 + 1] * SinDegrees((i / 3 + 1) / static_cast<float>(MAX_DEBRIS_SUBDIVISION) * 360.f),
			0.f),
			color,
			Vec2(0.f, 0.f)
		);
		mesh[i + 1] = Vertex(Vec3(randomLengths[i / 3] * CosDegrees((i / 3) / static_cast<float>(MAX_DEBRIS_SUBDIVISION) * 360.f),
			randomLengths[i / 3] * SinDegrees((i / 3) / static_cast<float>(MAX_DEBRIS_SUBDIVISION) * 360.f),
			0.f),
			color,
			Vec2(0.f, 0.f)
		);
		mesh[i + 2] = Vertex(Vec3(0.f, 0.f, 0.f),
			color,
			Vec2(0.f, 0.f)
		);
	}


	mesh[MAX_DEBRIS_SUBDIVISION * 3 - 3] = Vertex(Vec3(randomLengths[0], 0.f, 0.f),
		color,
		Vec2(0.f, 0.f)
	);
	mesh[MAX_DEBRIS_SUBDIVISION * 3 - 2] = Vertex(Vec3(randomLengths[MAX_DEBRIS_SUBDIVISION - 1] *
		CosDegrees((MAX_DEBRIS_SUBDIVISION - 1) / static_cast<float>(MAX_DEBRIS_SUBDIVISION) * 360.f),
		randomLengths[MAX_DEBRIS_SUBDIVISION - 1] * SinDegrees((MAX_DEBRIS_SUBDIVISION - 1) / static_cast<float>(MAX_DEBRIS_SUBDIVISION) * 360.f),
		0.f),
		color,
		Vec2(0.f, 0.f)
	);
	mesh[MAX_DEBRIS_SUBDIVISION * 3 - 1] = Vertex(Vec3(0.f, 0.f, 0.f),
		color,
		Vec2(0.f, 0.f)
	);
}

//-------------------------------------------------------------------------------------------------
void Debris::SetColorDuringLifeTime() {
	for (int i = 0; i < MAX_DEBRIS_SUBDIVISION * 3; i++) {
		m_localMesh[i].m_color.a = static_cast<unsigned char>(Interpolate(0.f, 255.f, m_lifeTimeCur / m_lifeTime));
	}
}