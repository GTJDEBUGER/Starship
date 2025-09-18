#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Game/BeetleEnemy.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Playership.hpp"
#include "Game/Game.hpp"

//-----------------------------------------------------------------------------------------------
BeetleEnemy::BeetleEnemy(Game* game)
	: Entity(game, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y))
{
	m_physicsRadius = ENEMY_BEETLE_PHYSICS_RADIUS;
	m_cosmeticRadius = ENEMY_BEETLE_COSMETIC_RADIUS;
	m_curFrame = 0;
	m_maxFrame = 2;
#pragma region Local Mesh Defination
	// Define the Beetle mesh (in local space)
	Rgba8 carapaceColor = Rgba8(161, 67, 5, 255);
	Rgba8 muscleColor = Rgba8(245, 153, 92, 255);
	Rgba8 wingColor = Rgba8(233, 233, 233, 128);
	// Head_Base
	m_localMesh[0] = Vertex(Vec3(0.5f, 0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[1] = Vertex(Vec3(1.18f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[2] = Vertex(Vec3(0.5f, -0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Head_Horn_Base
	m_localMesh[3] = Vertex(Vec3(1.f, 0.24f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[4] = Vertex(Vec3(1.7f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[5] = Vertex(Vec3(1.f, -0.24f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Head_Horn_Up
	m_localMesh[6] = Vertex(Vec3(1.4f, 0.1f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[7] = Vertex(Vec3(1.94f, 0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[8] = Vertex(Vec3(1.7f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Head_Horn_Down
	m_localMesh[9] = Vertex(Vec3(1.4f, -0.1f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[10] = Vertex(Vec3(1.7f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[11] = Vertex(Vec3(1.94f, -0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Body_Base
	m_localMesh[12] = Vertex(Vec3(-1.5f, 0.f, 0.f), muscleColor, Vec2(0.f, 0.f));
	m_localMesh[13] = Vertex(Vec3(0.5f, 0.9f, 0.f), muscleColor, Vec2(0.f, 0.f));
	m_localMesh[14] = Vertex(Vec3(0.5f, -0.9f, 0.f), muscleColor, Vec2(0.f, 0.f));
	// Leg_Front_Left
	m_localMesh[15] = Vertex(Vec3(0.5f, 0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[16] = Vertex(Vec3(1.f, 1.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[17] = Vertex(Vec3(0.67f, 0.67f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Front_Left
	m_localMesh[18] = Vertex(Vec3(1.f, 1.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[19] = Vertex(Vec3(1.29f, 0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[20] = Vertex(Vec3(0.86f, 0.86f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Front_Right
	m_localMesh[21] = Vertex(Vec3(0.5f, -0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[22] = Vertex(Vec3(1.f, -1.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[23] = Vertex(Vec3(0.67f, -0.67f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Front_Right
	m_localMesh[24] = Vertex(Vec3(1.f, -1.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[25] = Vertex(Vec3(1.29f, -0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[26] = Vertex(Vec3(0.86f, -0.86f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Left
	m_localMesh[27] = Vertex(Vec3(-0.4f, 0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[28] = Vertex(Vec3(-0.8f, 0.31f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[29] = Vertex(Vec3(-1.1f, 0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Left
	m_localMesh[30] = Vertex(Vec3(-1.1f, 0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[31] = Vertex(Vec3(-0.8f, 0.31f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[32] = Vertex(Vec3(-1.11f, 0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Left
	m_localMesh[33] = Vertex(Vec3(-1.1f, 0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[34] = Vertex(Vec3(-1.11f, 0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[35] = Vertex(Vec3(-1.8f, 0.49f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Right
	m_localMesh[36] = Vertex(Vec3(-0.4f, -0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[37] = Vertex(Vec3(-0.8f, -0.31f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[38] = Vertex(Vec3(-1.1f, -0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Right
	m_localMesh[39] = Vertex(Vec3(-1.1f, -0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[40] = Vertex(Vec3(-0.8f, -0.31f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[41] = Vertex(Vec3(-1.11f, -0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Right
	m_localMesh[42] = Vertex(Vec3(-1.1f, -0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[43] = Vertex(Vec3(-1.11f, -0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[44] = Vertex(Vec3(-1.8f, -0.49f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Wing_Left_Front
	m_localMesh[45] = Vertex(Vec3(0.5f, 0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[46] = Vertex(Vec3(0.5f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[47] = Vertex(Vec3(-0.5f, 1.94f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Wing_Right_Front
	m_localMesh[48] = Vertex(Vec3(0.5f, -0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[49] = Vertex(Vec3(0.5f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	m_localMesh[50] = Vertex(Vec3(-0.5f, -1.94f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Wing_Left_Back_Frame_1
	m_localMesh[51] = Vertex(Vec3(0.5f, 0.f, 0.f), wingColor, Vec2(0.f, 0.f));
	m_localMesh[52] = Vertex(Vec3(-1.3f, 1.51f, 0.f), wingColor, Vec2(0.f, 0.f));
	m_localMesh[53] = Vertex(Vec3(0.f, 0.97f, 0.f), wingColor, Vec2(0.f, 0.f));
	// Wing_Right_Back_Frame_1
	m_localMesh[54] = Vertex(Vec3(0.5f, 0.f, 0.f), wingColor, Vec2(0.f, 0.f));
	m_localMesh[55] = Vertex(Vec3(-1.3f, -1.51f, 0.f), wingColor, Vec2(0.f, 0.f));
	m_localMesh[56] = Vertex(Vec3(0.f, -0.97f, 0.f), wingColor, Vec2(0.f, 0.f));
	// Wing_Left_Back_Frame_2
	m_localMesh[57] = Vertex(Vec3(0.5f, 0.f, 0.f), wingColor, Vec2(0.f, 0.f));
	m_localMesh[58] = Vertex(Vec3(-1.79f, 0.88f, 0.f), wingColor, Vec2(0.f, 0.f));
	m_localMesh[59] = Vertex(Vec3(0.f, 0.97f, 0.f), wingColor, Vec2(0.f, 0.f));
	// Wing_Right_Back_Frame_2
	m_localMesh[60] = Vertex(Vec3(0.5f, 0.f, 0.f), wingColor, Vec2(0.f, 0.f));
	m_localMesh[61] = Vertex(Vec3(-1.79f, -0.88f, 0.f), wingColor, Vec2(0.f, 0.f));
	m_localMesh[62] = Vertex(Vec3(0.f, -0.97f, 0.f), wingColor, Vec2(0.f, 0.f));
#pragma endregion

}

//-----------------------------------------------------------------------------------------------
void BeetleEnemy::Update(float deltaSeconds)
{
	m_curFrame = (m_curFrame + 1) % m_maxFrame;
	//--------------------------------------------------------------------------------
	Vec2 directionToPlayer = (m_game->m_player->m_position - m_position).GetNormalized();
	m_velocity = directionToPlayer * ENEMY_BEETLE_SPEED;
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees = directionToPlayer.GetOrientationDegrees();

}

//-----------------------------------------------------------------------------------------------
void BeetleEnemy::Render() const
{
	Vertex m_worldMesh[57];
	for (int i = 0; i < 57; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
	}
	if (m_curFrame) {
		for (int i = 51; i < 57; i++) {
			m_worldMesh[i] = m_localMesh[i+6];
		}
	}
	TransformVertexArrayXY3D(57, m_worldMesh, 1.f, m_orientationDegrees, m_position);
	g_engine->m_renderer->DrawVertexArray(57, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void BeetleEnemy::Die()
{
	m_isDead = true;
}